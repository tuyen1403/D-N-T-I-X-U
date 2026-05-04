DỰ ÁN: PHÂN TÍCH VÀ MÔ PHỎNG CHIẾN THUẬT QUẢN LÝ VỐN TRONG TRÒ CHƠI TÀI XỈU 

0. CÁCH CHƠI
   a. Dụng cụ 
    Sử dụng 3 viên xúc xắc (xí ngầu), mỗi viên có 6 mặt đánh số từ 1 đến 6.
   Cách thức: Người chơi đặt cược vào kết quả tổng số điểm của 3 viên xúc xắc sau khi nhà cái lắc .
   b. Hai cửa cược chính
   Cửa Xỉu: Thắng khi tổng điểm 3 viên xúc xắc từ 4 đến 10.
   Cửa Tài: Thắng khi tổng điểm 3 viên xúc xắc từ 11 đến 17.
   c. Đặc điểm
   Tính may rủi cao: Kết quả hoàn toàn dựa trên sự ngẫu nhiên của các viên xúc xắc.
   Nhịp độ nhanh: Mỗi ván chơi thường diễn ra rất ngắn, chỉ trong khoảng 1 phút.
Với cách chơi đơn giản tài xỉu luôn thu hút nhiều lượt người chơi, đặc biệt với hình thức online chỉ với một chiếc điện thoại hay máy tính bạn có thể chơi ở bất kỳ đâu và bất kỳ thời điểm nào
1. Giới thiệu: Từ trò chơi may rủi đến mô hình toán học Tài xỉu (Over/Under) không chỉ đơn thuần là việc dự đoán tổng số điểm của 03 viên xúc xắc. Trong dự án này, chúng ta không xây dựng một trò chơi để giải trí; chúng ta xây dựng một Hệ thống giả lập tự động (Simulation Engine). Hệ thống này có khả năng thực hiện hàng vạn ván cược trong tích tắc để phân tích hành vi của vốn (bankroll) dưới tác động của các thuật toán đặt cược khác nhau
2. Phân tích luật chơi của code
   a.Cơ chế vận hành cơ bản
    Dụng cụ: Hệ thống mô phỏng sử dụng 3 viên xúc xắc (Dice).  
    Cách tính điểm: Mỗi vòng đấu, 3 viên xúc xắc sẽ được gieo ngẫu nhiên để lấy tổng điểm (từ 3 đến 18).
   Quy tắc thắng/thua:
     Tài (Tai): Tổng điểm từ 11 đến 18.  
     Xỉu (Xiu): Tổng điểm từ 3 đến 10.  
  Người chơi chọn một trong hai cửa này. Nếu tổng điểm rơi vào đúng cửa đã chọn, người chơi thắng; ngược lại là thua.
  b. Quy định tài chính (Nhà cái và Người chơi)
  Tỷ lệ thắng: Mã nguồn thiết lập xác suất thắng cơ bản là 50% cho mỗi cửa.  Phí hoa hồng (Commission): Để duy trì bàn chơi, nhà cái thu phí 5% trên số tiền thắng của người chơi. Ví dụ: Nếu bạn thắng 100$, bạn chỉ nhận được 95$ tiền lãi thực tế.
  Giới hạn mức cược: Nhà cái áp đặt mức cược tối thiểu (min_bet) và tối đa (max_bet). Nếu người chơi đặt cược vượt quá hoặc thấp hơn mức này, số tiền cược sẽ tự động bị điều chỉnh về giới hạn của nhà cái.
  Điều kiện dừng: Vòng chơi sẽ kết thúc nếu số dư của người chơi (balance) về bằng hoặc nhỏ hơn 0.
3. CHIẾN THUẬT TỐI ƯU ĐỐI VỚI NGƯỜI CHƠI
   Chúng ta đưa vào "phòng thí nghiệm" 03 thuật toán kinh điển để kiểm chứng qua hàng ngàn vòng đấu:
   3.1. Martingale – Cơn ác mộng của sự tăng trưởng hàm mũ
   Cơ chế: Nhân đôi mức cược sau mỗi ván thua (current_bet * 2) và quay về mức cơ bản khi thắng.
   Phân tích: Trong code, Martingale đối mặt với rủi ro "cháy túi" cực nhanh nếu gặp chuỗi thua dài vì số tiền cược tăng theo cấp số nhân, dễ dàng chạm mức max_bet của nhà cái.
   3.2. Fibonacci – Sự phòng thủ mang tính toán học
   Cơ chế: Tiền cược tăng/giảm theo dãy số Fibonacci (1, 1, 2, 3, 5, 8...). Khi thắng, người chơi lùi 2 bậc trong dãy số; khi thua, tiến 1 bậc.
   Phân tích: Đây là chiến thuật bảo thủ hơn, giúp quỹ đạo vốn sụt giảm chậm hơn Martingale trong các chuỗi đen đủi do đó sự phòng thủ ở chiến thuật này vẫn chưa tốt nên vẫn chưa được coi là tối ưu nhất
    3.3. Paroli – Tối ưu hóa chuỗi thắng (Lựa chọn của chuyên gia)
   Cơ chế: Ngược lại với Martingale, Paroli chỉ tăng tiền cược khi thắng (current_bet * 2) và quay lại mức cược gốc sau một số ván thắng mục tiêu hoặc khi thua.
   Phân tích: Chiến thuật này tận dụng "tiền của nhà cái" để gia tăng lợi nhuận, đồng thời bảo vệ tiền vốn ban đầu một cách tối đa.( LỰA CHỌN TỐI ƯU NHẤT)
4. CHIẾN THUẬT TỪ NHÀ CÁI
   -Thu phế (Commission): Với tỷ lệ 5% trên mỗi ván thắng, nhà cái luôn có thể hưởng được một số tiền hoa hồng nhỏ từ đó không chỉ vậy họ nhờ vào việc cài đặt mức cược trên bàn chơi để có thể :
    a. Vô hiệu hóa chiến thuật Gấp thếp (Martingale)
      Chiến thuật Martingale dựa trên lý thuyết: "Nếu bạn thua, hãy đặt cược gấp đôi ở ván sau, cuối cùng bạn sẽ thắng và thu hồi lại toàn bộ vốn lẫn lãi".
      Nếu không có mức cược tối đa: Một người chơi có vốn cực lớn có thể theo đuổi chuỗi thua mãi mãi cho đến khi thắng.
      Khi có max_bet: Nhà cái chặn đứng khả năng nhân đôi này. Khi chuỗi thua đủ dài khiến số tiền cược ván tiếp theo vượt quá max_bet, người chơi buộc phải dừng lại hoặc chỉ được cược ở mức tối đa. Lúc này, người chơi không còn khả năng gỡ gạc lại những gì đã mất, dẫn đến việc "cháy túi"
    b.Kích hoạt định lý "Sự sụp đổ của con bạc" 
      Toán học chứng minh rằng trong một trò chơi có xác suất gần như bằng nhau, bên nào có nguồn vốn hữu hạn sẽ luôn thua bên có nguồn vốn (gần như) vô hạn trong dài hạn.
      Bằng cách giới hạn mức cược, nhà cái buộc người chơi phải chia nhỏ vốn và chơi nhiều ván hơn.
      Càng chơi nhiều ván, tỷ lệ thắng thực tế sẽ càng tiến gần về tỷ lệ lý thuyết (50/50). Tuy nhiên, vì nhà cái còn có phí hoa hồng 5% (COMMISSION = 0.05), nên càng chơi lâu, vốn của người chơi sẽ càng bị bào mòn và chảy về túi nhà cái.
    c.Kiểm soát rủi ro thanh khoản (Liquidity Risk)
      Mọi sòng bạc hay nhà cái đều có một hạn mức tài chính nhất định.
      Nếu một Player đặt cược một số tiền khổng lồ vượt quá khả năng chi trả của nhà cái trong một ván duy nhất, nhà cái sẽ đối mặt với nguy cơ phá sản (sập tiệm) nếu người chơi đó thắng.
      max_bet giúp nhà cái đảm bảo rằng dù người chơi có thắng, số tiền đó vẫn nằm trong giới hạn chi trả an toàn của họ\
5.SỬ DỤNG OOP MỘT CÁCH HỢP LÝ
 Áp dụng các mẫu thiết kế (Design Patterns)OOP mở đường cho việc sử dụng các Design Patterns chuyên nghiệp. Trong code của nhóm mình, việc sử dụng Strategy Pattern giúp hệ thống cực kỳ linh hoạt. Ta có thể dễ dàng thay đổi thuật toán đặt cược cho hàng ngàn người chơi ảo chỉ bằng cách thay đổi đối tượng chiến thuật được truyền vào.
6. Quản lý độ phức tạp: Khi dự án phát triển lên hàng ngàn ván đấu với nhiều loại chiến thuật khác nhau, cách tiếp cận theo hàm (Functional) thông thường sẽ khiến code trở nên rất khó kiểm soát (Spaghetti Code). OOP tổ chức code thành các khối cấu trúc, giúp bạn dễ dàng đọc, hiểu và bảo trì dự án trong dài hạn.
