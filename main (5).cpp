#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
constexpr double WIN_RATE  = 0.5;
constexpr double COMMISSION = 0.05;

// ─────────────────────────────────────────────
//  Enums & Bet
// ─────────────────────────────────────────────
enum class BetType   { Tai, Xiu };
enum class BetResult { Win, Lose };

class Bet {
public:
    Bet(BetType t, double a) : type(t), amount(a) {}
    BetType type;
    double  amount;
};

// ─────────────────────────────────────────────
//  ABettingStrategy
// ─────────────────────────────────────────────
class ABettingStrategy {
public:
    virtual ~ABettingStrategy() = default;
    virtual Bet  calcNextBet(BetResult const &) = 0;
    virtual void reset() { current_bet = base_bet; }
    double getBaseBet() const { return base_bet; }

protected:
    ABettingStrategy(double base) : base_bet(base), current_bet(base) {}
    double base_bet, current_bet;
};

// ─────────────────────────────────────────────
//  Martingale
// ─────────────────────────────────────────────
class Martingale : public ABettingStrategy {
public:
    Martingale(double base) : ABettingStrategy(base) {}

    Bet calcNextBet(BetResult const &prev) override {
        current_bet = (prev == BetResult::Win) ? base_bet : current_bet * 2;
        BetType side = (rand() % 2 == 0) ? BetType::Xiu : BetType::Tai;
        return Bet(side, current_bet);
    }
};

// ─────────────────────────────────────────────
//  Paroli
// ─────────────────────────────────────────────
class Paroli : public ABettingStrategy {
    int consecutive_wins;
    int target_wins;
public:
    Paroli(double base, int target) : ABettingStrategy(base),
        consecutive_wins(0), target_wins(target) {}

    Bet calcNextBet(BetResult const &prev) override {
        if (prev == BetResult::Win) {
            consecutive_wins++;
            if (consecutive_wins >= target_wins) {
                current_bet = base_bet;
                consecutive_wins = 0;
            } else {
                current_bet *= 2;
            }
        } else {
            current_bet = base_bet;
            consecutive_wins = 0;
        }
        BetType side = (rand() % 2 == 0) ? BetType::Xiu : BetType::Tai;
        return Bet(side, current_bet);
    }

    void reset() override {
        ABettingStrategy::reset();
        consecutive_wins = 0;
    }
};

// ─────────────────────────────────────────────
//  Fibonacci
// ─────────────────────────────────────────────
class Fibonacci : public ABettingStrategy {
    int current_idx;

    int getFibMultiplier(int n) {
        if (n <= 0) return 1;
        if (n == 1) return 1;
        int a = 1, b = 1, c = 2;
        for (int i = 2; i <= n; i++) { c = a + b; a = b; b = c; }
        return c;
    }
public:
    Fibonacci(double base) : ABettingStrategy(base), current_idx(0) {}

    Bet calcNextBet(BetResult const &prev) override {
        if (prev == BetResult::Win) {
            current_idx -= 2;
            if (current_idx < 0) current_idx = 0;
        } else {
            current_idx++;
        }
        current_bet = base_bet * getFibMultiplier(current_idx);
        BetType side = (rand() % 2 == 0) ? BetType::Xiu : BetType::Tai;
        return Bet(side, current_bet);
    }

    void reset() override {
        ABettingStrategy::reset();
        current_idx = 0;
    }
};

// ─────────────────────────────────────────────
//  Participant
// ─────────────────────────────────────────────
class Participant {
public:
    virtual ~Participant() = default;
    double getBalance() const          { return balance; }
    void   setBalance(double a)        { balance = a; }
    void   updateBalance(double delta) { balance += delta; }
protected:
    double balance = 0.0;
};

// ─────────────────────────────────────────────
//  House
// ─────────────────────────────────────────────
class House : public Participant {
    double commission_rate, min_bet, max_bet;
public:
    House(double cr, double mn, double mx)
        : commission_rate(cr), min_bet(mn), max_bet(mx) { balance = 0.0; }

    double getCommissionRate() const { return commission_rate; }
    double getMinBet()         const { return min_bet; }
    double getMaxBet()         const { return max_bet; }

    double limitBet(double amount) const {
        if (amount < min_bet) return min_bet;
        if (amount > max_bet) return max_bet;
        return amount;
    }
};

// ─────────────────────────────────────────────
//  Player
// ─────────────────────────────────────────────
class Player : public Participant {
    unique_ptr<ABettingStrategy> strategy;
    Bet current_bet;
public:
    Player(double init) : current_bet(BetType::Xiu, 0.0) { setBalance(init); }
    Player(Player &&) = default;

    Bet getCurrentBet() const { return current_bet; }

    void setStrategy(unique_ptr<ABettingStrategy> s) {
        strategy = move(s);
        strategy->reset();
        BetType side = (rand() % 2 == 0) ? BetType::Xiu : BetType::Tai;
        current_bet = Bet(side, strategy->getBaseBet());
    }

    void placeBet(BetResult const &prev) {
        if (strategy) current_bet = strategy->calcNextBet(prev);
    }
};

// ─────────────────────────────────────────────
//  Dice
// ─────────────────────────────────────────────
class Dice {
public:
    int roll() { return (rand() % 6) + 1; }
};

// ─────────────────────────────────────────────
//  RoundRecord
// ─────────────────────────────────────────────
class RoundRecord {
public:
    Bet    bet;
    BetResult result;
    double current_bankroll;
};

// ─────────────────────────────────────────────
//  Table
// ─────────────────────────────────────────────
class Table {
    vector<Dice>       dices;
    House              house;
    Player             player;
    vector<RoundRecord> records;
public:
    Table(House const &h, Player &&p)
        : dices(3), house(h), player(move(p)) {}

    void play() {
        if (player.getBalance() <= 0) return;

        int total = 0;
        for (auto &d : dices) total += d.roll();
        bool isTai = (total >= 11);

        Bet bet = player.getCurrentBet();
        bet.amount = house.limitBet(bet.amount);
        if (bet.amount > player.getBalance())
            bet.amount = player.getBalance();

        BetResult result;
        if ((isTai  && bet.type == BetType::Tai) ||
            (!isTai && bet.type == BetType::Xiu))
            result = BetResult::Win;
        else
            result = BetResult::Lose;

        if (result == BetResult::Win) {
            double profit = bet.amount * (1.0 - house.getCommissionRate());
            player.updateBalance(profit);
            house.updateBalance(-profit);
        } else {
            player.updateBalance(-bet.amount);
            house.updateBalance(bet.amount);
        }

        records.push_back({bet, result, player.getBalance()});
        player.placeBet(result);
    }

    vector<RoundRecord> const &getRecords() const { return records; }
};

// ─────────────────────────────────────────────
//  Over_under_betting_Exporter
// ─────────────────────────────────────────────
class Over_under_betting_Exporter {
    string path;
public:
    Over_under_betting_Exporter(string const &p) : path(p) {}

    void exportToCSV(const string &strategy_name,
                     const vector<RoundRecord> &records,
                     double initial_bankroll,
                     bool write_header = false) {
        ofstream file(path, write_header ? ios::out : ios::app);
        if (!file.is_open()) return;
        if (write_header)
            file << "strategy,round,side,amount,result,bankroll,roi\n";
        int round = 1;
        for (auto const &r : records) {
            double roi = (r.current_bankroll - initial_bankroll)
                         / initial_bankroll * 100.0;
            file << strategy_name << "," << round++ << ","
                 << (r.bet.type == BetType::Xiu ? "Xiu" : "Tai") << ","
                 << r.bet.amount << ","
                 << (r.result == BetResult::Win ? "Win" : "Lose") << ","
                 << r.current_bankroll << ","
                 << fixed << setprecision(2) << roi << "%\n";
        }
    }
};

// ─────────────────────────────────────────────
//  SimulationEngine
// ─────────────────────────────────────────────
using StrategyFactory = function<unique_ptr<ABettingStrategy>()>;

class SimulationEngine {
    int    total_rounds;
    double initial_bankroll, base_bet, max_bet;
    vector<pair<string, StrategyFactory>>          strategies;
    vector<pair<string, vector<RoundRecord>>>       results;

    vector<RoundRecord> runOne(StrategyFactory &factory) {
        House  house(0.05, base_bet, max_bet);
        Player player(initial_bankroll);
        player.setStrategy(factory());
        Table table(house, move(player));
        for (int i = 0; i < total_rounds; i++) table.play();
        return table.getRecords();
    }

public:
    SimulationEngine(int r, double bk, double mn, double mx)
        : total_rounds(r), initial_bankroll(bk), base_bet(mn), max_bet(mx) {}

    void addStrategy(string const &name, StrategyFactory f) {
        strategies.emplace_back(name, move(f));
    }

    void run() {
        results.clear();
        unsigned int seed = random_device{}();
        for (auto &[name, factory] : strategies) {
            srand(seed);
            results.emplace_back(name, runOne(factory));
        }
    }

    bool exportCSV(string const &filename, double initial_bankroll) const {
        Over_under_betting_Exporter exp(filename);
        bool first = true;
        for (auto const &[name, records] : results) {
            exp.exportToCSV(name, records, initial_bankroll, first);
            first = false;
        }
        return true;
    }
};


// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main(int /*argc*/, char *argv[]) {
    // CSV luôn nằm cùng thư mục với file thực thi
    fs::path exeDir  = fs::absolute(fs::path(argv[0])).parent_path();
    fs::path csvPath = exeDir / "results.csv";

    cout << "===== TAI XIU SIMULATION =====" << endl;

    int    rounds;
    double bankroll, minBet, maxBet;

    cout << "Nhap so van choi: ";  cin >> rounds;
    cout << "Nhap von ban dau: ";  cin >> bankroll;
    cout << "Nhap minBet: ";       cin >> minBet;
    cout << "Nhap maxBet: ";       cin >> maxBet;

    vector<pair<string, StrategyFactory>> availableStrategies = {
        {"Martingale", [minBet] { return make_unique<Martingale>(minBet); }},
        {"Paroli",     [minBet] { return make_unique<Paroli>(minBet, 3); }},
        {"Fibonacci",  [minBet] { return make_unique<Fibonacci>(minBet); }},
    };

    SimulationEngine engine(rounds, bankroll, minBet, maxBet);

    int numPlayers;
    cout << "\nNhap so luong nguoi choi: ";
    cin >> numPlayers;

    cout << "\nCac chien thuat duoc ap dung :" << endl;
    for (int i = 0; i < (int)availableStrategies.size(); i++)
        cout << "  " << i + 1 << ". " << availableStrategies[i].first << endl;

    for (int i = 1; i <= numPlayers; i++) {
        int choice;
        cout << "Player " << i << " chon (1-" << availableStrategies.size() << "): ";
        cin >> choice;
        choice--;
        if (choice < 0 || choice >= (int)availableStrategies.size()) {
            cout << "  Khong hop le, dung " << availableStrategies[0].first << " mac dinh." << endl;
            choice = 0;
        }
        string name = "Player" + to_string(i) + "_" + availableStrategies[choice].first;
        engine.addStrategy(name, availableStrategies[choice].second);
    }

    cout << "\nDang chay mo phong..." << endl;
    engine.run();

    string csvStr = csvPath.string();
    if (engine.exportCSV(csvStr, bankroll))
        cout << "Xuat file thanh cong: " << csvStr << endl;
    else
        cout << "Xuat file that bai!" << endl;

    return 0;
}
