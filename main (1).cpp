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
#include <map>
#include <numeric>
#include <algorithm>

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
//  RunStats — kết quả thống kê 1 chiến thuật
// ─────────────────────────────────────────────
struct RunStats {
    string         name;
    double         avg_final    = 0;
    double         median_final = 0;
    double         stdev        = 0;
    double         avg_peak     = 0;
    double         avg_low      = 0;
    double         avg_winrate  = 0;
    double         avg_survival = 0;
    double         bust_pct     = 0;
    double         profit_pct   = 0;
    double         roi          = 0;
    vector<double> avg_curve;   // bankroll TB theo từng ván
};

// ─────────────────────────────────────────────
//  MultiRunEngine
//  Chạy N lần, mỗi lần dùng đúng SimulationEngine
//  logic bên trong (House, Player, Table, Dice...)
// ─────────────────────────────────────────────
class MultiRunEngine {
    int    total_rounds;
    double initial_bankroll, base_bet, max_bet;
    int    num_runs;

    // Tái sử dụng đúng logic runOne của SimulationEngine
    vector<RoundRecord> runOne(StrategyFactory &factory, unsigned int seed) {
        srand(seed);
        House  house(COMMISSION, base_bet, max_bet);
        Player player(initial_bankroll);
        player.setStrategy(factory());
        Table  table(house, move(player));
        for (int i = 0; i < total_rounds; i++) table.play();
        return table.getRecords();
    }

    static double calcMedian(vector<double> v) {
        sort(v.begin(), v.end());
        int n = (int)v.size();
        if (n == 0) return 0;
        return n % 2 == 0 ? (v[n/2-1] + v[n/2]) / 2.0 : v[n/2];
    }

    static double calcStdev(const vector<double> &v, double mean) {
        if (v.size() < 2) return 0;
        double acc = 0;
        for (auto x : v) acc += (x - mean) * (x - mean);
        return sqrt(acc / (v.size() - 1));
    }

public:
    MultiRunEngine(int rounds, double bankroll, double minBet,
                   double maxBet, int runs = 200)
        : total_rounds(rounds), initial_bankroll(bankroll),
          base_bet(minBet), max_bet(maxBet), num_runs(runs) {}

    RunStats runStrategy(const string &name, StrategyFactory factory) {
        vector<vector<RoundRecord>> all_runs(num_runs);

        for (int r = 0; r < num_runs; r++) {
            unsigned int seed = random_device{}();
            all_runs[r] = runOne(factory, seed);

            // Progress bar
            int pct  = (r + 1) * 100 / num_runs;
            int bars = pct / 2;
            cout << "\r  [" << string(bars, '#')
                 << string(50 - bars, ' ')
                 << "] " << setw(3) << pct << "%  " << name
                 << " (" << r + 1 << "/" << num_runs << ")  ";
            cout.flush();
        }
        cout << "\r" << string(80, ' ') << "\r";

        // Tính thống kê
        RunStats s;
        s.name = name;

        vector<double> finals, peaks, lows, win_rates, survivals;
        for (auto const &run : all_runs) {
            if (run.empty()) continue;
            double final_bk = run.back().current_bankroll;
            double peak = initial_bankroll, low = initial_bankroll;
            int    wins = 0;
            for (auto const &rec : run) {
                if (rec.current_bankroll > peak) peak = rec.current_bankroll;
                if (rec.current_bankroll < low)  low  = rec.current_bankroll;
                if (rec.result == BetResult::Win) wins++;
            }
            finals.push_back(final_bk);
            peaks.push_back(peak);
            lows.push_back(low);
            win_rates.push_back(!run.empty()
                ? wins * 100.0 / run.size() : 0.0);
            survivals.push_back(run.size() * 100.0 / total_rounds);
        }

        auto mean = [](const vector<double> &v) {
            return v.empty() ? 0.0
                : accumulate(v.begin(), v.end(), 0.0) / v.size();
        };

        s.avg_final    = mean(finals);
        s.median_final = calcMedian(finals);
        s.stdev        = calcStdev(finals, s.avg_final);
        s.avg_peak     = mean(peaks);
        s.avg_low      = mean(lows);
        s.avg_winrate  = mean(win_rates);
        s.avg_survival = mean(survivals);
        s.bust_pct     = count_if(finals.begin(), finals.end(),
                             [](double f){ return f <= 0; })
                         * 100.0 / (double)finals.size();
        s.profit_pct   = count_if(finals.begin(), finals.end(),
                             [this](double f){ return f > initial_bankroll; })
                         * 100.0 / (double)finals.size();
        s.roi          = (s.avg_final - initial_bankroll)
                         / initial_bankroll * 100.0;

        // Bankroll curve TB theo từng ván
        s.avg_curve.assign(total_rounds, 0.0);
        vector<int> counts(total_rounds, 0);
        for (auto const &run : all_runs) {
            for (int v = 0; v < (int)run.size(); v++) {
                s.avg_curve[v] += run[v].current_bankroll;
                counts[v]++;
            }
        }
        for (int v = 0; v < total_rounds; v++)
            if (counts[v] > 0) s.avg_curve[v] /= counts[v];

        return s;
    }

    // Xuất bảng thống kê tóm tắt ra CSV
    bool exportStatCSV(const string &path,
                       const vector<RunStats> &stats) const {
        ofstream f(path, ios::out);
        if (!f.is_open()) return false;
        f << "strategy,avg_final,median_final,stdev,avg_peak,avg_low,"
          << "avg_winrate,avg_survival,bust_pct,profit_pct,roi\n";
        for (auto const &s : stats) {
            f << s.name << ","
              << fixed << setprecision(2)
              << s.avg_final    << "," << s.median_final << ","
              << s.stdev        << "," << s.avg_peak     << ","
              << s.avg_low      << "," << s.avg_winrate  << ","
              << s.avg_survival << "," << s.bust_pct     << ","
              << s.profit_pct   << "," << s.roi          << "\n";
        }
        return true;
    }

    // Xuất đường cong bankroll TB theo từng ván ra CSV
    bool exportCurveCSV(const string &path,
                        const vector<RunStats> &stats) const {
        ofstream f(path, ios::out);
        if (!f.is_open()) return false;
        f << "round";
        for (auto const &s : stats) f << "," << s.name;
        f << "\n";
        for (int v = 0; v < total_rounds; v++) {
            f << v + 1;
            for (auto const &s : stats)
                f << "," << fixed << setprecision(2)
                  << (v < (int)s.avg_curve.size() ? s.avg_curve[v] : 0.0);
            f << "\n";
        }
        return true;
    }

    // In bảng tóm tắt ra console
    void printSummary(const vector<RunStats> &stats) const {
        const string sep(74, '=');
        const string sep2(74, '-');
        cout << "\n" << sep << "\n";
        cout << "  KET QUA MO PHONG " << num_runs
             << " LAN x " << total_rounds << " VAN\n";
        cout << sep << "\n";
        cout << left
             << setw(22) << "Chien thuat"
             << setw(10) << "BK TB"
             << setw(9)  << "ROI"
             << setw(10) << "WinRate"
             << setw(10) << "Song sot"
             << setw(10) << "Chay tui"
             << "Co lai\n";
        cout << sep2 << "\n";
        for (auto const &s : stats) {
            cout << left
                 << setw(22) << s.name
                 << setw(10) << fixed << setprecision(1) << s.avg_final
                 << setw(8)  << fixed << setprecision(1) << s.roi         << "% "
                 << setw(8)  << fixed << setprecision(1) << s.avg_winrate << "% "
                 << setw(8)  << fixed << setprecision(1) << s.avg_survival<< "% "
                 << setw(8)  << fixed << setprecision(1) << s.bust_pct    << "% "
                 << fixed    << setprecision(1) << s.profit_pct << "%\n";
        }

        // Xếp hạng
        auto ranked = stats;
        sort(ranked.begin(), ranked.end(),
             [](const RunStats &a, const RunStats &b) {
                 double sa = a.roi*0.5 + a.avg_winrate*0.3
                           + a.avg_survival*0.2 - a.bust_pct*2;
                 double sb = b.roi*0.5 + b.avg_winrate*0.3
                           + b.avg_survival*0.2 - b.bust_pct*2;
                 return sa > sb;
             });
        const vector<string> medals = {"[1] NHAT", "[2] NHI ", "[3] BA  "};
        cout << "\n" << sep << "\n";
        cout << "  XEP HANG TOI UU\n";
        cout << sep << "\n";
        for (int i = 0; i < (int)ranked.size(); i++) {
            string medal = i < (int)medals.size()
                ? medals[i] : "[" + to_string(i+1) + "]     ";
            double score = ranked[i].roi * 0.5
                         + ranked[i].avg_winrate * 0.3
                         + ranked[i].avg_survival * 0.2
                         - ranked[i].bust_pct * 2;
            cout << "  " << medal << "  " << left << setw(26) << ranked[i].name
                 << "Diem: " << fixed << setprecision(2) << score;
            if (i == 0) cout << "  <-- TOI UU NHAT";
            cout << "\n";
        }
        cout << sep << "\n";
    }
};

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
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

    // ── Mô phỏng nhiều lần: 200 lần x rounds ván ──────────────────
    cout << "\nBat dau mo phong nhieu lan (200 lan x "
         << rounds << " van)..." << endl;

    MultiRunEngine multiEngine(rounds, bankroll, minBet, maxBet, 200);
    vector<RunStats> allStats;

    for (auto const &[name, factory] : availableStrategies) {
        cout << "  " << name << "..." << endl;
        StrategyFactory f = factory;
        allStats.push_back(multiEngine.runStrategy(name, f));
    }

    multiEngine.printSummary(allStats);

    fs::path statPath  = exeDir / "simulation_stats.csv";
    fs::path curvePath = exeDir / "simulation_curve.csv";

    if (multiEngine.exportStatCSV(statPath.string(), allStats))
        cout << "\nXuat thong ke: " << statPath.string() << endl;
    if (multiEngine.exportCurveCSV(curvePath.string(), allStats))
        cout << "Xuat duong cong: " << curvePath.string() << endl;

    // ── Gọi Python script để vẽ biểu đồ ───────────────────────────
    cout << "\nDang goi Python de ve bieu do..." << endl;

    // Đảm bảo Python script chạy trong đúng thư mục chứa file CSV
    // Lưu ý: Đặt tên file python của bạn là 'plot.py'
    fs::path pyScriptPath = exeDir / "plot.py";

    // cd tới thư mục chứa file thực thi và gọi python
    // Đối với Windows thường dùng "python", đối với macOS/Linux có thể cần dùng "python3"
    std::string command = "cd \"" + exeDir.string() + "\" && python \"" + pyScriptPath.string() + "\"";

    int result = std::system(command.c_str());

    if (result == 0) {
        cout << "Hoan thanh! Bieu do da duoc tao va luu thanh cong." << endl;
    } else {
        cout << "Loi khi chay Python (Ma loi: " << result << ").\n";
        cout << "Vui long kiem tra:\n";
        cout << "1. File 'plot.py' co nam cung thu muc khong?\n";
        cout << "2. Ban da cai dat Python va cac thu vien chua? (pip install pandas matplotlib numpy)\n";
        cout << "3. Neu dung macOS/Linux, hay thu thay chu 'python' bang 'python3' trong code C++.\n";
    }

    return 0;
}
