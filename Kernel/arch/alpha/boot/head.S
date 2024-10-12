/* SPDX-License-Identifier: GPL-2.0 */
/*
 * arch/alpha/boot/head.S
 *
 * initial bootloader stuff..
 */

/* đoạn mã này thuộc kiến trúc MIPS trong linux kernel và có tác dụng trong việc khởi tạo hệ thống khi boot. */
#include <asm/pal.h> /* đây là chỉ thị để bao gồm một file header liên quan đến các định nghĩa cho lệnh PAL (Privileged Architecture Library), sử dụng trong các kiến trúc như Alpha hoặc MIPS. PAL chứa các chức năng dành cho quản lý các thao tác đặc quyền trên các kiến trúc này. */

	.set noreorder /* chỉ thị này yêu cầu trình biên dịch assembler (trình biên dịch mã hợp ngữ) không sắp xếp lại các lệnh sau đó. trên kiến trúc MIPS, việc reorder (sắp xếp lại lệnh) có thể xảy ra để tối ưu  hóa hiệu năng, nhưng ở một số phần quan trọng như mã boot, ta không muốn điều đó, vì cần giữ thứ tự lệnh chính xác để hệ thống khởi động đúng. */
	.globl	__start /* đây là chỉ thị để khai báo một nhãn label __start ở phạm vi toàn cầu, có thể truy cập từ các file mã nguồn khác __start là điểm  khởi đầu của hệ thống sau khi hệ thống phần cứng hoàn tất việc thiết lập. */
	.ent	__start /* chỉ thị này đánh dấu sự bắt đầu của một thủ tục hoặc hàm có tên là __start. đây là vị trí chính xác nơi CPU sẽ bắt đầu thực thi mã sau khi khởi động. các thủ tục được khai báo bằng .ent thường kèm theo .end để xác định phạm vi của hàm. */
	/* Đây là phần mã khởi động của kernel trên kiến trúc MIPS. __start sẽ là điểm bắt đầu của quá trình khởi tạo kernel khi máy khởi động */
/* đoạn mã này cũng thuộc phần khởi động của linux kernel trên kiến trúc MIPS hoặc Alpha, nơi kernel bắt đầu khởi tạo. */
__start: /* đây là nhãn label __start, nơi CPU bắt đầu thực thi sau khi khởi động. nó được khai báo trước đó và đây là nơi phần mã khởi động bắt đầu. */
	br	$29,2f /* đây là lệnh branch (nhảy) đến nhãn 2:. br(branch) là lệnh nhảy có điều kiện, và $29 là một thanh ghi CPU, cụ thể là thanh ghi lưu địa chỉ quay lại (return address) trên alpha. trong ngữ cảnh này, $29 chứa giá trị của một thanh ghi quan trọng và quá trình nhảy này nhằm chuẩn bị trước cho các bước tiếp theo */
2:	ldgp	$29,0($29) /* lệnh ldgp là load global pointer, dùng để tải con trỏ toàn cục (global pointer) vào thanh ghi $29 (trong Alpha, thanh ghi $29 cũng gọi là GP - global pointer). đây là bước thiết lập các con trỏ để truy cập các biến toàn cục trong suốt quá trình khởi động. */
	jsr	$26,start_kernel /* jsr (jump to subroutine) là lệnh nhảy tới một hàm con, trong trường hợp này là start_kernel. thay ghi $26 sẽ được sử dụng để lưu địa chỉ trở về (return address), khi kết thúc hàm start_kernel. hàm start_kernel là nơi quá trình khởi tạo chính của linux kernel bắt đầu, bao gồm thiết lập các thành phần của hệ thống như bộ nhớ, CPU, và các driver thiết bị. */
	call_pal PAL_halt /* call_pal là một lệnh đặc quyền dùng trên Alpha để gọi các lệnh PAL (Privileged Architecture Library). ở đây, PAL_halt sẽ dừng CPU nếu quá trình khởi động thất bại hoặc sau khi đã hoàn tất nhiệm vụ. nếu kernel không thể khởi động đúng cách, lệnh này sẽ dừng toàn bộ hệ thống. */
	.end __start /* kết thúc thủ tục __start. Đây là chỉ thị cho assembler để kết thúc định nghĩa của nhãn __start */

	.align 5 /* căn chỉnh mã theo ranh giới 32-byte (vì 2^5 = 32). việc căn chỉnh giúp tối ưu hóa việc truy cập bộ nhớ, tránh lỗi hoặc giảm thiểu việc sử dụng bộ nhớ không hiệu quả. */
	.globl	wrent /* khai báo nhãn wrent là một biểu tượng toàn cục để có thể được truy cập từ các file mã nguồn khác. */
	.ent	wrent /* đánh dấu sự bắt đầu của thủ tục wrent, tương tự như __start. mặc dù đoạn mã chi tiết của wrent chưa được đưa ra, nhưng đây có thể là một thủ tục hoặc một hàm quan trọng khác trong quá trình khởi động. */
	/* Đoạn mã này khởi tạo các thông số cần thiết trước khi nhảy vào hàm chính start_kernel, nơi toàn bộ hệ điều hành được thiết lập và bắt đầu vận hành. Lệnh call_pal PAL_halt là bước dự phòng để dừng hệ thống trong trường hợp khởi động thất bại. */

/* đoạn mã này tiếp tục thuộc phần khởi động của linux kernel trên kiến trúc Alpha */
wrent: /* đây là nhãn wrent, đánh dấu bắt đầu của một thủ tục hoặc hàm con */
	.prologue 0 /* chỉ thị này cho assembler biết rằng thủ tục wrent không cần thực hiện bất kỳ bước thiết lập thông thường nào (như lưu trạng thái thanh ghi), tức là không có phần "prologue". điều này thường xảy ra khi hàm này thực hiện một hành động rất đơn giản, không yêu cầu lưu trữ nhiều thông tin. */
	call_pal PAL_wrent /* đây là lệnh đăc  quyền call_pal gọi PAL(Privileged Architecture Library) với mã PAL_wrent. lệnh PAL_wrent (write entitlement) trong hệ thống Alpha có tác dụng thiết lập quyền ghi (write Privilege) của CPU hoặc điều chỉnh các thông số đặc quyền cụ thể cho hệ thống. nó có thể được sử dụng để cập nhật hoặc thay đổi quyền truy cập vào các tài nguyên hệ thống. */
	ret ($26) /* lệnh ret(return) sử dụng giá trị trong thanh ghi $26 để quay trở lại địa chỉ gọi hàm trước đó. $26 là thanh ghi lưu trữ địa chỉ quay lại (return address) khi thực hiện lệnh jsr trước đó. lệnh này hoàn tất quá trình thực thi của wrent và đưa CPU trở lại vị trí gọi hàm. */
	.end wrent /* kết thúc thủ tục wrent, báo cho assembler rằng đây là phần cuối của hàm wrent */ 

	.align 5 /* căn chỉnh mã theo ranh giới 32-byte, giúp tối ưu hóa truy cập bộ nhớ. */
	.globl	wrkgp /* khai báo nhãn wrkgp là một biểu tượng toàn cục, có thể được truy cập từ các file mã nguồn khác. */
	.ent	wrkgp /* đánh dấu sự bắt đầu của một thủ tục hoặc hàm wrkgp. đoạn mã chi tiết của wrkgp chưa được cung cấp, nhưng nó sẽ chứa một hàm có tác dụng cụ thể trong quá trình khởi động hệ thống. */
	/* Thủ tục wrent được sử dụng để gọi lệnh PAL đặc quyền PAL_wrent, giúp điều chỉnh hoặc thiết lập quyền truy cập đặc quyền của CPU đối với một số tài nguyên của hệ thống. Đây là một phần của quá trình khởi tạo hệ thống trên kiến trúc Alpha, đảm bảo rằng các quyền được thiết lập chính xác trước khi kernel tiếp tục vận hành. */

/* đoạn mã này tiếp tục thuộc phần khởi động của linux kernel trên kiến trúc Alpha */	
wrkgp: /* đây là nhãn wrkgp, đánh dấu bắt đầu của một thủ tục hoặc hàm con */
	.prologue 0 /* chỉ thị này cho assembler biết rằng thủ tục wrkgp không có phần khởi tạo trước (prologue), nghĩa là không có bất kỳ lưu trữ thanh ghi hay thiết lập ngữ cảnh nào cần thiết cho hàm này. */
	call_pal PAL_wrkgp /* lệnh này gọi hàm PAL (Privileged Architecture Library) với mã lệnh PAL_wrkgp. đây là một lệnh đặc quyền trong hệ thống alpha, và nó có thể liên quan đến việc ghi (write) hoặc thao tác với thanh ghi con trỏ toàn cục (GP - Global Pointer). PAL_wrkgp thường dùng để cập nhật con trỏ toàn cục hoặc thực hiện những thao tác đặc quyền khác liên quan đến quản lý trạng thái hệ thống. */
	ret ($26) /* lệnh ret (return) sử dụng giá trị trong thanh ghi $26 để quay trở lại địa chỉ của lệnh gọi trước đó. thanh ghi $26 giữ địa chỉ quay lại sau khi thực hiện lệnh jsr trong các thủ tục gọi hàm */
	.end wrkgp /* kết thúc thủ tục wrkgp, báo cho assembler rằng đây là phần cuối của hàm wrkgp */

	.align 5 /* căn chỉnh mã theo ranh giới 32byte, nhằm tối ưu hóa hiệu năng khi truy cập bộ nhớ. */
	.globl	switch_to_osf_pal /* khai báo nhãn switch_to_osf_pal là biểu tượng toàn cục, có thể được truy cập từ các file mã nguồn khác. */
	.ent	switch_to_osf_pal /* đánh dấu sự bắt đầu của một thủ tục hoặc hàm switch_to_osf_pal. tuy đoạn mã chi tiết của switch_to_osf_pal chưa được cung cấp, nhưng nó có thể là một thủ tục quan trọng trong quá trình chuyển đổi giữa các trạng thái hoặc chế độ khác nhau của hệ thống. */
	/* Thủ tục wrkgp được sử dụng để gọi lệnh PAL đặc quyền PAL_wrkgp, có khả năng tương tác hoặc ghi giá trị vào thanh ghi con trỏ toàn cục (Global Pointer). Đây là một bước quan trọng trong quá trình khởi động và thiết lập trạng thái hệ thống, đặc biệt trong việc quản lý bộ nhớ và ngữ cảnh hệ thống trên kiến trúc Alpha. */

/* Đoạn mã này thực hiện quá trình lưu trữ trạng thái của các thanh ghi trên CPU vào ngăn xếp stack trước khi hệ thống thực hiện một hành động quan trọng, có thể là chuyển sang chế độ OSF PAL (Privileged Architecture Library) trên kiến trúc Alpha */
switch_to_osf_pal: /* đây là nhãn, đánh dấu vị trí bắt đầu của hàm switch_to_osf_pal */
	subq	$30,128,$30 /* trừ đi 128 byte từ thanh ghi $30 (stack pointer - con trỏ ngăn xếp), sau đó lưu kết quả vào chính $30. điều này tạo không gian trên ngăn xếp để lưu trữ các thanh ghi hiện tại của CPU. việc này giúp bảo toàn trạng thái trước khi thực hiện các thao tác khác. */
	.frame	$30,128,$26 /* đây là chỉ thị cho assembler biết về cấu trúc của stack frame hiện tại. $30 là con trỏ ngăn xếp, 128 là kích thước của stack frame (đã được tạo ở lệnh trên), và $26 là thanh ghi lưu địa chỉ quay lại (return address). điều này giúp đảm bảo rằng quá trình gọi hàm và quay lại sau đó được thực hiện đúng cách. */
	stq	$26,0($30) /* lưu thanh ghi $26 (địa chỉ quay lại) vào vị trí đầu tiên của ngăn xếp. $26 giữ địa chỉ cần quay lại sau khi hàm kết thúc, vì vậy cần lưu trữ nó an toàn trước khi thực hiện các thao tác khác. */
	stq	$1,8($30) /* lưu các thanh ghi $1 đến $15 vào các vị trí tiếp theo trên ngăn xếp (mỗi thanh ghi cách nhau 8 byte). đây là quá trình lưu toàn bộ trạng thái của CPU (tất cả các thanh ghi chính), đảm bảo chúng được bảo quản khi chuyển đổi ngữ cảnh hoặc chế độ. */
	stq	$2,16($30)
	stq	$3,24($30)
	stq	$4,32($30)
	stq	$5,40($30)
	stq	$6,48($30)
	stq	$7,56($30)
	stq	$8,64($30)
	stq	$9,72($30)
	stq	$10,80($30)
	stq	$11,88($30)
	stq	$12,96($30)
	stq	$13,104($30)
	stq	$14,112($30)
	stq	$15,120($30)
	.prologue 0 /* thông báo không có phần prologue bổ sung cần thiết. việc lưu các thanh ghi vào ngăn xếp đã hoàn thành, nên hàm đã sẵn sàng thực hiện các hành động tiếp theo. */
	/* Đoạn mã này là bước chuẩn bị trước khi chuyển đổi ngữ cảnh hoặc chế độ sang OSF PALcode (một lớp đặc quyền trong kiến trúc Alpha).
	Mục đích chính là lưu trạng thái CPU (các thanh ghi) vào ngăn xếp, nhằm bảo đảm rằng khi thực hiện thao tác đặc quyền hoặc chuyển đổi chế độ, hệ thống có thể khôi phục lại trạng thái ban đầu một cách an toàn.
	Đây là một phần quan trọng trong việc xử lý ngữ cảnh trong kernel, cho phép kernel chuyển đổi giữa các chế độ đặc quyền mà không mất dữ liệu. */

/* đoạn mã này thuộc về quá trình khôi phục ngữ cảnh CPU sau khi đã lưu trạng thái vào ngăn xếp. đây là một phần quan trọng trong cơ chế chuyển đổi ngữ cảnh hoặc quay trở lại từ một chế độ đặc quyền */
	stq	$30,0($17)	/* save KSP in PCB */ /* lưu giá trị của thanh ghi $30 (con trỏ ngăn xếp, stack pointer - SP) vào địa chỉ chứa trong thanh ghi $17. $17 ở đây có thể là một con trỏ trỏ tới PCB (Process Control Block - khối điều khiển tiến trình), nơi lưu trữ thông tin của một tiến trình. Điều này giúp lưu lại trạng thái của con trỏ ngăn xếp cho quá trình chuyển đổi ngữ cảnh. */

	bis	$30,$30,$20	/* a4 = KSP */ /* lệnh bis (bitwise inclusive OR) thực hiện phép toán OR giữa $30 và $30, kết quả là lưu lại giá trị của $30 vào $20. điều này giống như sao chép giá trị của $30 (KSP - kernel stack pointer) sang $20, và có thể sẽ được sử dụng trong các bước tiếp theo với mục đích thao tác với ngăn xếp. */
	br	$17,1f /* lệnh br (branch) nhảy tới địa chỉ được lưu trong thanh ghi $17. địa chỉ này có thể được tính toán từ trước, và nó sẽ đưa CPU đến nhãn 1f, có thể nằm ở một vị trí phía sau (thường là một nhãn tạm thời trong mã assembly) */

/* các lệnh ldq (load quadword) lần lượt nạp giá trị từ ngăn xếp (vị trí $30 + offset) vào các thanh ghi $26, $1, $2, ... $15. đây là quá trình khôi phục trạng thái các thanh ghi CPU từ ngăn xếp mà chúng đã được lưu trước đó. những giá trị này đã được lưu trong một stack frame trước khi chuyển sang chế độ đặc quyền, và giờ đang được khôi phục để quay lại trạng thái trước khi ngắt. */
	ldq	$26,0($30)
	ldq	$1,8($30)
	ldq	$2,16($30)
	ldq	$3,24($30)
	ldq	$4,32($30)
	ldq	$5,40($30)
	ldq	$6,48($30)
	ldq	$7,56($30)
	ldq	$8,64($30)
	ldq	$9,72($30)
	ldq	$10,80($30)
	ldq	$11,88($30)
	ldq	$12,96($30)
	ldq	$13,104($30)
	ldq	$14,112($30)
	ldq	$15,120($30)
	addq	$30,128,$30 /* cộng thêm 128 vào thanh ghi $30, đây là bước hoàn tác việc "đẩy" stack (hồi phục lại con trỏ ngăn xếp). khi khởi tạo stack frame trước đó, 128 byte đã được cấp phát trên ngăn xếp, giờ đang được "thu hồi" để trả lại trạng thái ban đầu. */
	ret ($26) /* lệnh ret(return) sử dụng giá trị trong thanh ghi $26 để quay trở lại địa chỉ lưu trong thanh ghi này. $26 chính là thanh ghi chứa địa chỉ quay lại, đã được khôi phục ở lệnh ldq $26, 0($30). điều này hoàn tất việc khôi phục ngữ cảnh và trả CPU về trạng thái trước đó. */
	/* Đoạn mã này khôi phục ngữ cảnh CPU từ ngăn xếp sau khi đã chuyển đổi chế độ hoặc thực hiện một thao tác đặc quyền. Điều này cần thiết khi hệ điều hành cần lưu trạng thái của một tiến trình (các thanh ghi CPU) trước khi chuyển sang tiến trình hoặc chế độ khác, sau đó quay lại tiếp tục xử lý tiến trình ban đầu mà không mất dữ liệu.
	Đây là một phần quan trọng của cơ chế context switching (chuyển đổi ngữ cảnh) trong kernel Linux, đảm bảo rằng hệ thống có thể chuyển đổi giữa các tiến trình hoặc chế độ mà không làm gián đoạn hoặc mất dữ liệu quan trọng của CPU. */

/* đoạn mã này tiếp tục phần kết thúc của hàm switch_to_osf_pal và bắt đầu khai báo một hàm mới tên là tbi. */
1:	call_pal PAL_swppal /* lệnh call_pal gọi một hàm đặc quyền PAL (Privileged Architecture Library) với mã lệnh PAL_swppal. đây là một lệnh đặc quyền chỉ có thể thực hiện ở chế độ kernel và có chức năng thay đổi trạng thái PAL(PALcode) của hệ thống. PAL_swppal thường được dùng để chuyển đổi giữa các chế độ hoạt động khác nhau của CPU (chẳng hạn từ PAL OSF sang PAL khác). lệnh này đánh dấu bước quan trọng trong việc chuyển đổi chế độ của hệ thống, có thể liên quan đến quản lý ngữ cảnh hoặc kiểm soát phần cứng trực tiếp. nhãn 1: đánh dấu vị trí trong mã mà CPU có thể nhảy đến khi cần, có thể trong quá trình thực thi khác hoặc các lệnh branch */
	.end	switch_to_osf_pal /* chỉ thị này thông báo kết thúc hàm switch_to_osf_pal. nó báo cho assembler rằng hàm này đã hoàn thành và không còn mã nào thuộc về hàm này nữa. */

	.align 3 /* chỉ thị này căn chỉnh mã theo ranh giới 8-byte (vì 3 là bội số của 8). việc căn chỉnh giúp cải thiện hiệu năng khi truy cập bộ nhớ, đặc biệt trên kiến trúc Alpha, nơi việc truy cập bộ nhớ cần được căn chỉnh để tối ưu tốc độ. */
	.globl	tbi /* khai báo nhãn tbi là biểu tượng toàn cục (global symbol), có thể được truy cập từ các file mã nguồn khác. điều này cho phép tbi có thể được gọi hoặc tham chiếu ở các đoạn mã khác trong kernel. */
	.ent	tbi /* đánh dấu bắt đầu của một thủ tục hoặc hàm mới có tên là tbi. tuy đoạn mã chi tiết của tbi chưa được cung cấp, nhưng với tên gọi "tbi" (có thể liên quan đến từ viết tắt của "translation buffer invalidate"), hàm này có thể liên quan đến việc thao tác với bộ nhớ ảo, đặc biệt là việc vô hiệu hóa một số mục trong bộ nhớ đệm dịch (translation buffer) của CPU */

	/* switch_to_osf_pal: Đoạn kết của hàm này thực hiện lệnh PAL_swppal, nhằm thay đổi chế độ hoạt động của hệ thống (thường là PALcode). Đây là một phần quan trọng trong quá trình khởi động hoặc chuyển đổi ngữ cảnh của CPU trên kiến trúc Alpha.

	tbi: Nhãn tbi chuẩn bị khai báo một hàm mới, nhiều khả năng sẽ xử lý thao tác liên quan đến bộ đệm dịch địa chỉ (translation buffer). Trong kernel Linux, việc thao tác với TLB (Translation Lookaside Buffer) và các thành phần liên quan đến quản lý bộ nhớ là rất quan trọng để đảm bảo hiệu năng và an toàn bộ nhớ khi chuyển đổi giữa các tiến trình hoặc thay đổi ánh xạ bộ nhớ ảo. */

/* đoạn mã này định nghĩa hàm tbi, thực hiện một lệnh đặc quyền liên quan đến thao tác với bộ đệm dịch địa chỉ (translation buffer), và chuẩn bị khai báo một hàm mới có tên là halt. */	
tbi: /* đây là nhãn cho hàm tbi, đánh dấu bắt đầu của hàm. */
	.prologue 0 /* chỉ thị này chỉ ra rằng hàm tbi không cần thêm phần prologue phức tạp (như lưu trạng thái hoặc các thanh ghi), điều này có nghĩa là hàm rất ngắn và thực hiện một số thao tác cụ thể, không cần chuẩn bị ngăn xếp. */
	call_pal PAL_tbi /* lệnh call_pal gọi một hàm PAL đặc quyền với mã lệnh PAL_tbi. PAL_tbi có thể liên quan đến việc thao tác với translation buffer invalidate (TBI), nhằm vô hiệu hóa hoặc xóa một số mục trong bộ đệm dịch địa chỉ (translation buffer), như TLB(translation Lookaside buffer), đây là một phần quan trọng trong quản lý bộ nhớ ảo. việc xóa bộ đệm dịch là cần thiết khi kernel cần đảm bảo rằng các mục cũ trong TLB không còn giá trị sau khi thay đổi ánh xạ bộ nhớ ảo, nhằm tránh tình trạng "dịch sai" hoặc truy cập không hợp lệ vào bộ nhớ. */
	ret	($26) /* lệnh ret sử dụng giá trị trong thanh ghi $26 để quay lại địa chỉ trước đó. $26 là thanh ghi chứa địa chỉ quay lại, và đây là cách tiêu chuẩn để kết thúc hàm trong assembly Alpha */
	.end tbi /* chỉ thị này đánh dấu kết thúc hàm tbi */

	.align 3 /* căn chỉnh mã theo ranh giới 8-byte (bội số của 8), đảm bảo mã được căn chỉnh đúng cách để CPU xử lý nhanh hơn. */
	.globl	halt /* khai báo nhãn halt là biểu tượng toàn cục, có thể được sử dụng từ các phần khác của mã */
	.ent	halt /* đánh dấu bắt đầu của hàm halt, một hàm mới có chức năng dừng hệ thống. tuy phần thân của hàm chưa được cung cấp, halt thường dùng để dừng mọi hoạt động của CPU hoặc đưa CPU vào trạng thái dừng. */
	/* tbi: Hàm tbi gọi lệnh PAL_tbi, một lệnh đặc quyền trên kiến trúc Alpha để thao tác với Translation Buffer Invalidate (TBI). Đây là thao tác quan trọng khi kernel cần làm mới các mục trong TLB (Translation Lookaside Buffer), giúp đảm bảo rằng bộ đệm dịch không còn chứa các ánh xạ cũ hoặc không hợp lệ sau khi thay đổi ánh xạ bộ nhớ.
	halt: Đoạn mã chuẩn bị khai báo hàm halt, thường dùng để dừng hệ thống. Trong kernel, halt thường được gọi khi hệ thống muốn dừng mọi hoạt động của CPU (ví dụ như khi tắt máy hoặc dừng hệ điều hành trong trường hợp khẩn cấp). */

/* đoạn mã này định nghĩa hàm halt, có nhiệm vụ dừng hoạt động của CPU bằng cách gọi lệnh đặc quyền PAL_halt. đây là một hàm khá đơn giản. */
halt: /* đây là nhãn đánh dấu bắt đầu hàm halt. nó định danh cho đoạn mã này và có thể được tham chiếu từ các phần khác trong chương trình. */
	.prologue 0 /* chỉ thị này chỉ ra rằng hàm không có phần prologue phức tạp (như lưu trữ trạng thái hoặc các thanh ghi). vì đây là hàm dừng hệ thống, không cần chuẩn bị thêm ngăn xếp hoặc bảo toàn trạng thái của CPU */
	call_pal PAL_halt /* lệnh này gọi lệnh đặc quyền PAL (Privileged Architecture Library) với mã lệnh PAL_halt. PAL_halt là một lệnh đặc quyền trên kiến trúc Alpha dùng để dừng hoàn toàn CPU. khi lệnh này được thực thi, CPU sẽ ngừng mọi hoạt động. đây thường là lệnh được gọi trong các trường hợp hệ thống cần dừng khẩn cấp hoặc tắt máy. */
	.end halt /* chỉ thị này thông báo kết thúc hàm halt. nó báo cho assembler rằng đoạn mã này đã hoàn tất. */
	/* halt: Hàm này có tác dụng dừng hệ thống bằng cách sử dụng lệnh PAL_halt, một lệnh đặc quyền trong kiến trúc Alpha. Trong kernel Linux, lệnh này thường được gọi khi hệ thống cần tắt hoặc đưa CPU vào trạng thái dừng hoàn toàn, chẳng hạn khi tắt máy hoặc trong trường hợp gặp sự cố nghiêm trọng mà hệ điều hành không thể tiếp tục hoạt động an toàn.
	Hàm này là thành phần quan trọng của hệ điều hành để xử lý các trường hợp dừng hoặc tắt hệ thống một cách hợp lệ. */

/* $16 - new stack page */
	.align 3 /* căn chỉnh mã theo ranh giới 8 byte (bội số của 8), tương đương với 2^3 = 8 byte. điều này giúp CPU truy cập bộ nhớ nhanh hơn và hiệu quả hơn. căn chỉnh đúng các hàm và dữ liệu trên bộ nhớ là rất quan trọng để tối ưu hiệu suất của CPU, đặc biệt trên các kiến trúc như Alpha. */
	.globl	move_stack /* khai báo nhãn move_stack là một biểu tượng toàn cục (global symbol), điều này có nghĩa là nó có thể được tham chiếu hoặc gọi từ các file mã nguồn khác. nhãn này định nghĩa hàm move_stack, được thiết kế để di chuyển ngăn xếp hoặc thao tác với bộ nhớ liên quan đến ngăn xếp. */
	.ent	move_stack /* chỉ thị này đánh dấu bắt đầu của hàm move_stack. điều này thông báo với assembler rằng đây là một hàm hoặc thủ tục có thể được gọi từ các phần khác trong chương trình. */

/* đoạn mã này định nghĩa hàm move_stack với các thao tác liên quan đến việc tính toán và cập nhật con trỏ ngăn xếp (stack pointer). */
move_stack: /* đây là nhãn đánh dấu bắt đầu của hàm move_stack */
	.prologue 0 /* chỉ thị này chỉ ra rằng không có phần prologue phức tạp trong hàm này (không cần chuẩn bị ngăn xếp hoặc bảo toàn trạng thái của các thanh ghi) */
	lda	$0, 0x1fff($31) /* lda (load address) là lệnh tải địa chỉ vào thanh ghi $0. lệnh này lấy giá trị 0x1fff và cộng thêm giá trị trong thanh ghi $31 (một thanh ghi đặc biệt, thường được sử dụng để giữ giá trị 0 trong nhiều kiến trúc). kết quả là giá trị 0x1fff được nạp vào thanh ghi $0. đây có thể là một giá trị "offset" liên quan đến kích thước của stack (8191 hoặc 0x1FFF tương ứng với 8kb ngăn xếp). lda có thể đang tính toán vị trí hoặc kích thước giới hạn của ngăn xếp. */
	and	$0, $30, $1			/* Stack offset */ /* lệnh and thực hiện phép AND giữa giá trị trong thanh ghi $0 (là 0x1fff) và giá trị trong thanh ghi $30 (con trỏ ngăn xếp hiện tại - stack pointer), sau đó lưu kết quả vào thanh ghi $1. mục đích của dòng lệnh này là lấy offset của ngăn xếp trong phạm vi 8kb bằng cách sử dụng phép AND với giá trị 0x1fff. điều này giới hạn địa chỉ ngăn xếp hiện tại trong một khoảng nhất định (có thể là giới hạn của page hoặc vùng bộ nhớ dành cho ngăn xếp). */
	or	$1, $16, $16			/* New stack pointer */ /* lệnh or thực hiện phép OR giữa giá trị trong thanh ghi $1 (kết quả phép AND trước đó) và thanh ghi $16. kết quả được lưu lại vào thanh ghi $16. dòng lệnh này có thể đang tính toán con trỏ ngăn xếp mới (new stack pointer) bằng cách hợp nhất giá trị offset đã tính toán từ $1 với một giá trị khác trong $16. */
	mov	$30, $1 /* lệnh mov sao chép giá trị của thanh ghi $30 (con trỏ ngăn xếp hiện tại) vào thanh ghi $1. điều này lưu giữ giá trị ngăn xếp cũ vào $1 để sử dụng sau. */
	mov	$16, $2 /* lệnh mov sao chép giá trị của thanh ghi $16 (con trỏ ngăn xếp mới sau khi thực hiện phép OR) vào thanh ghi $2. điều này chuẩn bị cho bước tiếp theo bằng cách lưu giữ giá trị của con trỏ ngăn xếp mới. */
	/* Tính toán và cập nhật con trỏ ngăn xếp: Mã này thực hiện việc tính toán con trỏ ngăn xếp mới bằng cách sử dụng offset được tính từ giá trị hiện tại của ngăn xếp. Sau đó, nó cập nhật con trỏ ngăn xếp với giá trị mới. Đây là một phần quan trọng của quản lý ngăn xếp, có thể trong quá trình chuyển đổi ngữ cảnh (context switching) hoặc khi kernel điều chỉnh ngăn xếp cho các tiến trình hoặc luồng mới.
	Quản lý bộ nhớ ngăn xếp: Việc sử dụng phép AND với 0x1fff có thể nhằm mục đích giới hạn địa chỉ ngăn xếp trong một phạm vi cụ thể, giúp đảm bảo rằng các tiến trình không vượt qua giới hạn ngăn xếp và vi phạm vùng bộ nhớ khác.
	Lưu trữ con trỏ ngăn xếp cũ và mới: Việc lưu giữ giá trị cũ của con trỏ ngăn xếp vào $1 và giá trị mới vào $2 có thể liên quan đến việc kernel giữ lại ngữ cảnh cũ và chuyển sang ngữ cảnh mới, một bước cần thiết trong việc chuyển đổi ngăn xếp hoặc tiến trình. */

/* đoạn mã này hoàn thiện hàm move_stack, với mục địch di chuyển dữ liệu từ một ngăn xếp cũ sang một ngăn xếp mới. */
1:	ldq	$3, 0($1)			/* Move the stack */ /* lệnh ldq (load quadword) nạp một từ 64-bit (quadword) từ địa chỉ được giữ bởi thanh ghi $1 vào thanh ghi $3. cụ thể, nó đang lấy giá trị từ địa chỉ ngăn xếp cũ (được giữ trong $1) và lưu trữ giá trị đó vào thanh ghi $3 */
	addq	$1, 8, $1 /* lệnh addq (add quadword) cộng thêm 8 vào thanh ghi $1 (địa chỉ ngăn xếp cũ) và lưu kết quả lại vào $1. điều này giúp di chuyển con trỏ $1 đến vị trí của phần tử tiếp theo trong ngăn xếp (vì mỗi phần tử trong ngăn xếp chiếm 8 byte). */
	stq	$3, 0($2) /* lệnh stq (store quadword) lưu giá trị 64bit trong thanh ghi $3 vào địa chỉ ngăn xếp mới (được giữ bởi $2). như vậy, giá trị vừa được lấy từ ngăn xếp cũ đã được ghi vào ngăn xếp mới. */
	and	$0, $1, $4 /* lệnh and thực hiện phép and giữa giá trị trong thanh ghi $0 (giá trị 0x1fff từ đoạn mã trước, nhằm giới hạn ngăn xếp) và $1 (địa chỉ hiện tại của ngăn xếp cũ), lưu kết quả vào $4. mục đích là giới hạn con trỏ ngăn xếp cũ trong phạm vi 8kb, đảm bảo rằng việc sao chép ngăn xếp vẫn nằm trong giới hạn hợp lệ. */
	addq	$2, 8, $2 /* lệnh này cộng thêm 8 vào thanh ghi $2 (địa chỉ ngăn xếp mới) và lưu kết quả vào $2. nó di chuyển con trỏ $2 đến vị trí tiếp theo trong ngăn xếp mới, tương tự như cách di chuyển con trỏ ngăn xếp cũ $1. */
	bne	$4, 1b /* bne (branch if not equal) kiểm tra giá trị của thanh ghi $4. nếu giá trị trong $4 không bằng 0, chương trình sẽ quay lại nhãn 1: để lặp lại quá trình di chuyển từ ngăn xếp cũ sang ngăn xếp mới. điều này tạo ra một vòng lặp, sao chép từng phần tử của ngăn xếp từ ngăn xếp cũ sang ngăn xếp mới cho đến khi đạt giới hạn (được xác định bởi phép AND trước đó). */
	mov	$16, $30 /* lệnh mov sao chép giá trị của thanh ghi $16 (được tính toán ở phần trước của hàm) vào thanh ghi $30 (stack pointer mới) điều này cập nhật con trỏ ngăn xếp của hệ thống sang ngăn xếp mới đã được sao chép. */
	ret	($26) /* lệnh ret sử dụng thanh ghi $26 để quay lại địa chỉ nơi hàm move_stack đã được gọi. */
	.end move_stack /* chỉ thị này báo hiệu kết thúc của hàm move_stack */

/* move_stack: Hàm này có tác dụng di chuyển toàn bộ ngăn xếp từ một vị trí cũ sang một vị trí mới. Đây là một phần quan trọng trong việc quản lý bộ nhớ và ngăn xếp trong kernel, đặc biệt trong quá trình chuyển đổi ngữ cảnh (context switching) hoặc khi cần thay đổi vị trí của ngăn xếp do các điều chỉnh trong hệ thống bộ nhớ (chẳng hạn như khi ánh xạ lại không gian địa chỉ).
Cơ chế sao chép ngăn xếp: Vòng lặp trong hàm liên tục sao chép dữ liệu từ ngăn xếp cũ sang ngăn xếp mới, cập nhật các con trỏ tương ứng cho đến khi toàn bộ ngăn xếp được sao chép. Sau đó, ngăn xếp của hệ thống được chuyển sang ngăn xếp mới, đảm bảo rằng mọi hoạt động tiếp theo sẽ sử dụng đúng ngăn xếp đã được cập nhật.
Việc quản lý ngăn xếp đúng cách là rất quan trọng để tránh tình trạng ghi đè bộ nhớ hoặc lỗi truy cập bộ nhớ, điều này giúp kernel hoạt động an toàn và ổn định. */