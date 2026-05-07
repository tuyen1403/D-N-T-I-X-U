import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np

# Đọc dữ liệu từ file CSV
df = pd.read_csv('results.csv')

# Thiết lập style cho biểu đồ
plt.style.use('seaborn-v0_8-whitegrid')
fig, ax = plt.subplots(figsize=(12, 7))

# Màu sắc cho từng chiến thuật
colors = {
    'Martingale': '#1f77b4',  # Xanh dương
    'Paroli': '#ff7f0e',       # Cam
    'Fibonacci': '#2ca02c',    # Xanh lá
}

# Lấy danh sách các chiến thuật (player)
strategies = df['strategy'].unique()

# Vẽ đường cho từng chiến thuật
for strategy in strategies:
    strategy_data = df[df['strategy'] == strategy]
    
    # Xác định màu dựa trên tên chiến thuật
    color = '#333333'  # Màu mặc định
    for key in colors:
        if key in strategy:
            color = colors[key]
            break
    
    ax.plot(strategy_data['round'], 
            strategy_data['bankroll'], 
            label=strategy, 
            linewidth=2,
            color=color,
            alpha=0.9)

# Lấy vốn ban đầu từ dữ liệu (giá trị bankroll đầu tiên + amount nếu thua hoặc - profit nếu thắng)
first_row = df.iloc[0]
if first_row['result'] == 'Win':
    initial_bankroll = first_row['bankroll'] / (1 + (1 - 0.05))  # Ước tính
else:
    initial_bankroll = first_row['bankroll'] + first_row['amount']

# Thử đọc từ file stats nếu có
try:
    stats_df = pd.read_csv('simulation_stats.csv')
    # Tính vốn ban đầu từ avg_final và roi
    if len(stats_df) > 0:
        avg_final = stats_df['avg_final'].iloc[0]
        roi = stats_df['roi'].iloc[0]
        initial_bankroll = avg_final / (1 + roi/100)
except:
    pass

# Nếu không tính được, dùng giá trị mặc định hợp lý
if initial_bankroll <= 0 or np.isnan(initial_bankroll):
    initial_bankroll = 1000000  # 1 triệu mặc định

# Vẽ đường vốn ban đầu
max_round = df['round'].max()
ax.axhline(y=initial_bankroll, color='red', linestyle='--', linewidth=2, 
           label=f'Vốn ban đầu ({initial_bankroll:,.0f})', alpha=0.8)

# Định dạng trục Y với đơn vị K (nghìn) hoặc Tr (triệu)
def format_currency(x, pos):
    if x >= 1000000:
        return f'{x/1000000:.1f} Tr'
    elif x >= 1000:
        return f'{x/1000:.0f} K'
    else:
        return f'{x:.0f}'

ax.yaxis.set_major_formatter(mticker.FuncFormatter(format_currency))

# Thiết lập nhãn và tiêu đề
ax.set_xlabel('Số ván cược (Round)', fontsize=12, fontweight='bold')
ax.set_ylabel('Số dư (Bankroll)', fontsize=12, fontweight='bold')
ax.set_title('Biến động Tài khoản theo từng Chiến thuật', fontsize=14, fontweight='bold', pad=15)

# Thiết lập legend
ax.legend(loc='upper left', fontsize=10, framealpha=0.9, fancybox=True, shadow=True)

# Thiết lập grid
ax.grid(True, linestyle='-', alpha=0.3)
ax.set_axisbelow(True)

# Thiết lập giới hạn trục
ax.set_xlim(0, max_round + 5)

# Tự động điều chỉnh giới hạn trục Y
y_min = df['bankroll'].min()
y_max = df['bankroll'].max()
y_range = y_max - y_min
ax.set_ylim(max(0, y_min - y_range * 0.1), y_max + y_range * 0.1)

# Thêm annotation cho điểm cao nhất và thấp nhất của mỗi chiến thuật
for strategy in strategies:
    strategy_data = df[df['strategy'] == strategy]
    
    # Điểm cao nhất
    max_idx = strategy_data['bankroll'].idxmax()
    max_round_val = strategy_data.loc[max_idx, 'round']
    max_bankroll = strategy_data.loc[max_idx, 'bankroll']
    
    # Điểm thấp nhất (không tính = 0)
    min_data = strategy_data[strategy_data['bankroll'] > 0]
    if len(min_data) > 0:
        min_idx = min_data['bankroll'].idxmin()
        min_round_val = min_data.loc[min_idx, 'round']
        min_bankroll = min_data.loc[min_idx, 'bankroll']

# Điều chỉnh layout
plt.tight_layout()

# Lưu biểu đồ
plt.savefig('bankroll_chart.png', dpi=150, bbox_inches='tight', 
            facecolor='white', edgecolor='none')

print("Đã tạo biểu đồ: bankroll_chart.png")

# Hiển thị biểu đồ
plt.show()
