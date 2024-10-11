// SPDX-License-Identifier: GPL-2.0
/*
 * arch/alpha/boot/bootpz.c
 *
 * Copyright (C) 1997 Jay Estabrook
 *
 * This file is used for creating a compressed BOOTP file for the
 * Linux/AXP kernel
 *
 * based significantly on the arch/alpha/boot/main.c of Linus Torvalds
 * and the decompression code from MILO.
 */
#include <linux/kernel.h> //bao gồm các khai báo cần thiết cho các chức năng liên quan đến kernel, chẳng hạn như in ra thông báo debug và quản lý bộ nhớ
#include <linux/slab.h> //cung cấp các hàm quản lý bộ nhớ, như kmalloc và kfree, cho phép cấp phát và giải phóng bộ nhớ trong kernel
#include <linux/string.h> //cung cấp các hàm xử lý chuỗi, như memcpy, memset, và strcmp
#include <generated/utsrelease.h> //bao gồm thông tin về phiên bản kernel(release version) đang biên dịch
#include <linux/mm.h> //chứa các khai báo liên quan đến quản lý bộ nhớ (memory management) trong kernel
#include <asm/console.h> //bao gồm các định nghĩa và hàm liên quan đến điều khiển console ở cấp độ assembly
#include <asm/hwrpb.h> //chứa định nghĩa cho HWRPB (hardware resource partition block), được sử dụng để lưu trữ thông tin về tài nguyên phần cứng.
#include <asm/io.h>//cung cấp các hàm để thực hiện các thao tác I/O ở cấp độ thấp chẳng hạn như đọc và ghi vào các thanh ghi phần cứng
#include <linux/stdarg.h> //bao gồm các định nghĩa liên quan đến xử lý đối số biến, thường được sử dụng trong các hàm như printf để hỗ trợ một số lượng không xác định các đối số
#include "kzsize.h" //đây có thể là một header tùy chỉnh được định nghĩa bởi người dùng hoặc trong dự án đang làm việc. nội dung của header này không thể biết được chỉ từ tên, nhưng nó có thể liên quan đến kích thước của các đối tượng trong kernel hoặc các cấu trúc dữ liệu.
//đoạn mã trên thiết lập các thư viện và header cần thiết cho việc phát triển trong không gian kernel của linux. các thư viện này cho phép thực hiện nhiều thao tác quan trọng, từ quản lý bộ nhớ đến xử lý I/O và quản lý chuỗi. việc bao gồm các header như utsrelease.h và hwrpb.h cũng cho phép mã truy cập vào thông tin cụ thể của hệ thống và phần cứng mà nó đang chạy.

/* FIXME FIXME FIXME */
//đoạn mã này làm một số việc cấp phát bộ nhớ và debug trong quá trình phát triển kernel
// Malloc_area_size được định nghĩa là 0x200000 (2MB), kích thước vùng bộ nhớ được sử dụng cho việc cấp phát động (dynamic memory allocation).
//0x200000 là một giá trị hex, tương ứng với 2^21 byte tức khoảng 2mb
// ghi chú dưới cũng cho biết đây là một giới hạn tạm thời và có thể sẽ cần điều chỉnh lại sau.
#define MALLOC_AREA_SIZE 0x200000 /* 2MB for now */
// FIXME đây là một lời nhắc rằng cần xem xét hoặc sửa lại một số điểm trong mã nguồn này. nó thường được sử dụng trong quá trình phát triển khi biết rằng một đoạn mã hoặc logic nào đó có thể không hoàn chỉnh hoặc cần tối ưu hóa.
/* FIXME FIXME FIXME */
// dưới là một ghi chú cảnh báo rằng việc bật thêm các thông báo có thể là thông báo debug có thể gây hỏng hình ảnh kernel do sử dụng quá nhiều bộ nhớ ngăn xếp stack.
// trong kernel, ngăn xếp thường có giới hạn về kích thước, và việc sử dụng nó quá mức để in ra các thông báo có thể dẫn đến việc ghi đè bộ nhớ quan trọng hoặc gây ra lỗi bộ nhớ.
/*
  WARNING NOTE

  It is very possible that turning on additional messages may cause
  kernel image corruption due to stack usage to do the printing.

*/
// hủy bỏ định nghĩa debug
// undef: các macro DEBUG_CHECK_RANGE, DEBUG_ADDRESSES, và DEBUG_LAST_STEPS bị loại bỏ (undefine), có nghĩa là những tính năng debug này sẽ không được sử dụng trong phiên bản mã này.
// có thể các macro này trước đó đã được định nghĩa ở một nơi khác và được sử dụng để kiểm tra địa chỉ, phạm vi, hoặc bước cuối cùng trong quy trình, nhưng chúng đã bị tắt trong đoạn mã này nhằm tránh in quá nhiều thông báo, điều có thể gây ra sự cố với bộ nhớ ngăn xếp như đã cảnh bảo ở trên.
#undef DEBUG_CHECK_RANGE
#undef DEBUG_ADDRESSES
#undef DEBUG_LAST_STEPS

// đoạn mã này cung cấp chứa các khai báo  của các hàm và biến liên quan đến việc chuyển đổi, giải nén kernel, di chuyển ngăn xếp, và các cấu trúc dữ liệu phần cứng (HWRPB - hardware resource partition block)
// đây là một hàm extern, có nghĩa là hàm này được định nghĩa ở một nơi khác, có thể là trong một tệp nguồn khác hoặc một thư viện đã biên dịch sẵn.
// mục đích là hàm này có khả năng thực hiện việc chuyển đổi sang chế độ PAL (privileged Architecture Library) OSF(Open software foundation) PAL là một thành phần của kiến trúc hệ thống Alpha và xử lý các lệnh đặc biệt ở cấp độ thấp
// tham số unsigned long nr là một tham số số nguyên dài, có thể là một số lệnh hoặc trạng thái
// struct pcb_struct * pcb_va là một con trỏ tới cấu trúc PCB(Process Control Block) lưu trữ địa chỉ ảo
// struct pcb_struct * pcb_pa là một con trỏ tới PCB lưu trữ địa chỉ vật lý.
// unsigned long *vptb là con trỏ tới bảng trang ảo (virtual page table base)
// kết quả trả về là hàm trả về một giá trị unsigned long, có thể là một mã trang thái hoặc kết quả của quá trình chuyển đổi.
extern unsigned long switch_to_osf_pal(unsigned long nr, struct pcb_struct * pcb_va, struct pcb_struct * pcb_pa, unsigned long *vptb);
// hàm này có nhiệm vụ giải nén hình ảnh kernel từ một nguồn (source) đến một đích (destination).
// với các tham số: void* destination: địa chỉ đích, nơi kernel sẽ được giải nén.
// void* source là địa chỉ nguồn chứa kernel đã nén
// size_t ksize: là kích thước của kernel nén.
// size_t kzsize là kích thước của kernel sau khi giải nén.
// kết quả trả về là hàm trả về một int, có thể là mã lỗi hoặc thành công của quá trình giải nén
extern int decompress_kernel(void* destination, void *source, size_t ksize, size_t kzsize);
// hàm này di chuyển ngăn xếp (stack) đến địa chỉ mới được chỉ định bởi new_stack
// với tham số unsigned long new_stack thì địa chỉ mới của ngăn xếp. điều này giúp đảm bảo rằng ngăn xếp không bị ghi đè hoặc can thiệp bởi các phần khác của bộ nhớ khi kernel được tải vào.
extern void move_stack(unsigned long new_stack);
// hwrpb là một con trỏ tới cấu trúc hwrpb_struct, đại diện cho hardware resource partition block. đây là một vùng bộ nhớ đặc biệt trên hệ thống Alpha lưu trữ thông tin về tài nguyên phần cứng.
// INIT_HWRPB có thể là một macro hoặc giá trị khởi tạo ban đầu cho cấu trúc này.
struct hwrpb_struct *hwrpb = INIT_HWRPB;
// đây là một mảng tĩnh chứa 1 phần tử của cấu trúc pcb_struct. cấu trúc này thường lưu trữ thông tin về process control block (khối điều khiển tiến trình), bao gồm các thông tin về trạng thái tiến trình, ngữ cảnh bộ nhớ, và các thông tin quan trọng khác.
// việc sử dụng mảng với chỉ một phần tử [1] gợi ý rằng đây có thể là cấu trúc dành riêng cho một tiến trình hoặc có thể sử dụng để lưu địa chỉ ảo của PCB
static struct pcb_struct pcb_va[1];
// switch_to_osf_pal: chuyển đổi sang chế độ pal osf, xử lý các ngữ cảnh về địa chỉ ảo và địa chỉ vật lý.
// decompress_kernel: giải nén kernel từ bộ nhớ nguồn đến địa chỉ đích.
// move_stack: Di chuyển ngăn xếp đến vị trí an toàn để tránh bị ghi đè.
// hwrpb: Trỏ đến thông tin về phần cứng hệ thống Alpha.
// pcb_va: Mảng tĩnh lưu trữ thông tin về Process Control Block.
// Đoạn mã trên là một phần quan trọng trong quá trình khởi động và quản lý kernel, liên quan đến việc tải và quản lý trạng thái hệ thống cấp thấp
/*
 * Find a physical address of a virtual object..
 *
 * This is easy using the virtual page table address.
 */
// đoạn mã cung cấp định nghĩa một macro và một hàm inline tĩnh trong C nhằm tìm địa chỉ vật lý (Physical Address) từ một địa chỉ ảo ( Vitual Address) thông qua việc sử dụng bảng trang ảo (vitual page table base - VPTB).
#define VPTB((unsigned long *) 0x200000000) // đây là một macro để định nghĩa bảng trang ảo (virtual page table base) tại địa chỉ ảo cố định 0x200000000. đây có thể là địa chỉ cơ sở của bảng trang ảo trong hệ thống.

static inline unsigned long find_pa(unsigned long address){ //hàm này nhận về một địa chỉ ảo và trả về địa chỉ vật lý tương ứng. Đây là một hàm nội tuyến tĩnh, có nghĩa là nó được định nghĩa bằng từ khóa inline static, nhằm mục đích tối ưu hóa bằng cách yêu cầu trình biên dịch chèn trực tiếp mã lệnh của hàm này tại vị trí nó được gọi, thay vì thực hiện lời gọi hàm thông thường.
	unsigned long result;
	result = VPTB[address >> 13]; //địa chỉ ảo bị dịch phải 13 bit, có thể là để lấy chỉ mục tương ứng trong bảng VPTB, gán kết quả là phần tử trong bảng VPTB tương ứng với địa chỉ ảo đã dịch.
	result >>= 32; //dịch kết quả sang phải 32bit, có thể là để loại bỏ các bit không cần thiết.
	result <<= 13; // dịch trái lại 13 bit để khôi phục lại phần địa chỉ vật lý.
	result |= address & 0x1fff; // kết hợp phần kết quả với các bit thấp của địa chỉ ảo, nhằm bảo đàm rằng các bit này được bảo toàn trong địa chỉ vật lý.
	return result;
}	

int check_range(unsigned long vstart, unsigned long vend, unsigned long kstart, unsigned long kend){ //hàm này thực hiện kiểm tra xem có sự chồng lấn giữa hai khoảng địa chỉ bộ nhớ hay không. 
//vstart, vend: đây là các giới hạn cho vùng địa chỉ ảo (vùng bắt đầu và kết thúc).
//kstart, kend: đây là các giới hạn cho vùng địa chỉ vật lý (vùng bắt đầu và kết thúc).
	unsigned long vaddr, kaddr;//biến vaddr: địa chỉ ảo, bắt đầu từ vstart và tăng dần theo kích thước trang nhớ (PAGE_SIZE). kaddr: địa chỉ vật lý, được tính từ địa chỉ ảo vaddr qua hàm find_pa(vaddr) và cộng thêm một offset (PAGE_OFFSET).
// nếu DEBUG_CHECK_RANGE được định nghĩa, hàm sẽ in ra thông tin kiểm tra về các khoảng địa chỉ bằng hàm srm_printk. điều này rất hữu ích khi debug để kiểm tra xem chương trình có đi đúng các bước dự kiến không.
#ifdef DEBUG_CHECK_RANGE
	srm_printk("check_range: V[0x%lx:0x%lx] K[0x%lx:0x%lx]\n",
		   vstart, vend, kstart, kend);
#endif
	/* do some range checking for detecting an overlap... */
	//vòng lặp for chạy qua các địa chỉ ảo từ vstart đến vend, bước nhảy bằng kích thước của một trang nhớ (PAGE_SIZE).
	// tại mỗi bước, nó tìm địa chỉ vật lý tương ứng với địa chỉ ảo và cộng thêm PAGE_OFFSET.
	for (vaddr = vstart; vaddr <= vend; vaddr += PAGE_SIZE){
		kaddr = (find_pa(vaddr) | PAGE_OFFSET);
		// kiểm tra chồng lấn (overlap):
		// sau khi tính toán kaddr, hàm kiểm tra xem địa chỉ vật lý này có nằm trong khoảng từ kstart đến kend hay không.
		// nếu có sự chồng lấn, hàm sẽ trả về giá trị 1, đồng thời in ra thông tin nếu có định nghĩa DEBUG_CHECK_RANGE
		// nếu không tìm thấy sự chồng lấn sau khi duyệt hết các địa chỉ ảo, hàm trả về 0.
		if (kaddr >= kstart && kaddr <= kend){
#ifdef DEBUG_CHECK_RANGE
			srm_printk("OVERLAP: vaddr 0x%lx kaddr 0x%lx"
				   " [0x%lx:0x%lx]\n",
				   vaddr, kaddr, kstart, kend);
#endif
			return 1;
		}
	}
	return 0;
}

/*
 * This function moves into OSF/1 pal-code, and has a temporary
 * PCB for that. The kernel proper should replace this PCB with
 * the real one as soon as possible.
 *
 * The page table muckery in here depends on the fact that the boot
 * code has the L1 page table identity-map itself in the second PTE
 * in the L1 page table. Thus the L1-page is virtually addressable
 * itself (through three levels) at virtual address 0x200802000.
 */

//đoạn mã là một phần của một hàm khởi tạo liên quan đến cấu trúc dữ liệu PCB (process control block) và PAL (processor abstraction layer) trong một hệ điều hành, có thể là trên kiến trúc alpha hoặc tương tự.
#define L1	((unsigned long *) 0x200802000) // L1 ở đây là 1 con trỏ tới địa chỉ cơ sở của một cấu trúc dữ liệu (có thể là bảng trang cấp 1) trong không gian bộ nhớ. Địa chỉ này có thể tương ứng với một phần cứng cụ thể hoặc một không gian địa chỉ được ánh xạ.

void pal_init(void){ //đây là hàm khởi tạo PAL-code. PAL-code là một tầng trừu tượng hóa phần cứng (tầng gần gũi với kiến trúc CPU) được sử dụng trong một số kiến trúc vi xử lý. nó có thể cung cấp các chức năng xử lý đặc biệt của bộ xử lý mà hệ điều hành có thể gọi trực tiếp.
	unsigned long i, rev;
	struct percpu_struct * percpu;
	struct pcb_struct * pcb_pa;

	/* Create the dummy PCB.  */
	//đây là việc thiết lập một pcb (process control block) giả (dummy pcb). pcb chứa các thông tin quan trọng cho việc chuyển đổi ngữ cảnh (context switching) như: ksp: kernel stack pointer, usp: user stack pointer, ptbr: physical table base register (trỏ tới bảng trang), asn, pcc, unique, flags: các trường liên quan đến quy trình xử lý.
	// đặc biệt ptbr được lấy từ giá trị của L1[1] và dịch sang 32 bit cao.
	pcb_va->ksp = 0;
	pcb_va->usp = 0;
	pcb_va->ptbr = L1[1] >> 32;
	pcb_va->asn = 0;
	pcb_va->pcc = 0;
	pcb_va->unique = 0;
	pcb_va->flags = 1;
	pcb_va->res1 = 0;
	pcb_va->res2 = 0;
	pcb_pa = (struct pcb_struct *)find_pa((unsigned long)pcb_va);

	/*
	 * a0 = 2 (OSF)
	 * a1 = return address, but we give the asm the vaddr of the PCB
	 * a2 = physical addr of PCB
	 * a3 = new virtual page table pointer
	 * a4 = KSP (but the asm sets it)
	 */
	// chuyển đổi sang PAL của OSF: đây là đoạn chuyển đối môi trường hiện tại sang sử dụng OSF PAL-code (OSF là một biến thể unix trên hệ điều hành Alpha). hàm switch_to_osf_pal sẽ thực hiện việc chuyển đổi, với các tham số bao gồm PCB và địa chỉ bảng trang.
	srm_printk("Switching to OSF PAL-code... ");

	i = switch_to_osf_pal(2, pcb_va, pcb_pa, VPTB);
	if (i) {
		srm_printk("failed, code %ld\n", i);
		__halt();
	}
	// đoạn mã này khởi tạo cấu trúc dữ liệu dành riêng cho từng CPU (per-CPU structure). cấu trúc này có chứa thông tin về phiên bản PAL-code mà CPU hiện đang sử dụng (pal_revision). dòng lệnh sau cùng in ra phiên bản PAL-code
	percpu = (struct percpu_struct *)
		(INIT_HWRPB->processor_offset + (unsigned long) INIT_HWRPB);
	rev = percpu->pal_revision = percpu->palcode_avail[2];

	srm_printk("OK (rev %lx)\n", rev);
	// tbia() có thể là một lệnh hủy bỏ toàn bộ bộ đệm (TLB- Translation Lookaside Buffer), dùng để làm mới việc ánh xạ trang trong trường hợp hệ thống đang chạy trong chế độ SMP (Symmetric Multiprocessing), để đảm bảo mọi CPU trong hệ thống đều có thể truy cập đúng bộ nhớ.
	tbia(); /* do it directly in case we are SMP */
	// Hàm pal_init này có chức năng chính là:
	// Khởi tạo một PCB (Process Control Block) giả.
	// Thiết lập bảng trang (page table) và chuyển sang PAL-code của OSF.
	// Thiết lập cấu trúc dữ liệu dành riêng cho CPU và ghi nhận phiên bản của PAL-code.
	// Hủy bỏ cache của bảng trang (TLB) nếu hệ thống đang chạy ở chế độ SMP.
}

/*
 * Start the kernel.
 */
// hàm runkernel này là một inline assembly trên một kiến trúc máy tính cụ thể (có thể là kiến trúc Alpha hoặc tương tự.), được sử dụng để nhảy vào kernel( hạt nhân hệ điều hành) và bắt đầu thực thi.
static inline void runkernel(void){
	// __asm__ __volatile__: đây là cú pháp cho inline assembly trong c. từ khóa __volatile__ đảm bảo rằng trình biên dịch sẽ không tối ưu hóa hoặc bỏ qua đoạn mã assembly này, do nó có các tác động trực tiếp đến trạng thái của cpu
	// bis %0,%0,$27: lệnh bis (bitwise inclusive OR) trong kiến trúc Alpha là một lệnh để sao chép dữ liệu giữa các thanh ghi. nó thực hiện phép toán OR trên hai toán hạng, nhưng ở đây, nó chỉ sử dụng một toán hạng %0 và thực hiện OR với chính nó, kết quả không thay đổi.
	// %0: là một vị trí thay thế cho giá trị START_ADDR, được truyền vào từ C và được lưu trong một thanh ghi.
	// $27: là một thanh ghi cụ thể (có thể là t12 trong kiến trúc Alpha). lệnh này sao chép giá trị của START_ADDR vào thành ghi $27.
	// jmp ($27): lệnh này nhảy đến địa chỉ được lưu trong thanh ghi $27, tức là nó sẽ nhảy đến địa chỉ của kernel được lưu trong START_ADDR và bắt đầu thực thi từ đó.
	__asm__ __volatile__(
		"bis %0,%0,$27\n\t" //đưa giá trị START_ADDR vào thanh ghi $27
		"jmp ($27)" //nhảy tới địa chỉ được lưu trong $27
		: /* no outputs: it doesn't even return */
		// "r" (START_ADDR): đây là cách START_ADDR (một biến được xác định trước trong mã C) được truyền vào đoạn mã assembly như là một toán hạng đầu vào. biến này sẽ được ánh xạ vào một thanh ghi CPU bởi trình biên dịch.
		// phần chú thích nhấn mạnh rằng đoạn mã này không có đầu ra vì lệnh jmp ($27) sẽ chuyển quyền kiểm soát cho kernel và sẽ không trở lại hàm runkernel
		: "r" (START_ADDR)
	); //START__ADDR là input operand được nạp vào thanh ghi.
	// Hàm runkernel là một đoạn mã dùng để nhảy đến kernel và bắt đầu thực thi từ địa chỉ được lưu trong biến START_ADDR. Cụ thể:
	// Nó sao chép giá trị START_ADDR vào thanh ghi $27.
	// Sau đó, nó sử dụng lệnh jmp để nhảy đến địa chỉ được lưu trong $27 (chính là START_ADDR), khởi chạy kernel.
	// START_ADDR là địa chỉ của kernel, có thể được định nghĩa trước.
	// jmp ($27) là lệnh nhảy không điều kiện, chuyển quyền điều khiển đến kernel và sẽ không quay lại.
	// Đoạn mã này được sử dụng trong các hệ thống yêu cầu nhảy trực tiếp vào không gian kernel sau khi khởi tạo đầy đủ môi trường làm việc.
}

/* Must record the SP (it is virtual) on entry, so we can make sure
   not to overwrite it during movement or decompression. */
// là một khai báo biến trong C. biến này có tên là SP_on_entry và kiểu data là unsigned long, tức là một số nguyên không dấu (có kích thước lớn hơn hoặc bằng 32 bit, tùy thuộc vào kiến trúc hệ thống, thường là 64bit trên các kiến trúc 64 bit);
unsigned long SP_on_entry;

/* Calculate the kernel image address based on the end of the BOOTP
   bootstrapper (ie this program).
*/
extern char _end; //khai báo biến ngoài _end, được định nghĩa ở một nơi khác trong mã nguồn, biến này thường được dùng để chỉ ra cuối cùng của đoạn mã và dữ liệu trong bộ nhớ.
#define KERNEL_ORIGIN \ ((((unsigned long)&_end) + 511) & ~511) //macro này tính toán địa chỉ gốc của kernel, sử dụng biến _end. nó làm tròn địa chỉ lên đến biên giới trang kế tiếp, bằng cách thêm 511 (để đảm bảo làm tròn lên) và sau đó áp dụng bitwise AND với ~511 để loại bỏ các bit lẻ, chỉ để lại các biên giới trang.

/* Round address to next higher page boundary. */
#define NEXT_PAGE(a)	(((a) | (PAGE_SIZE - 1)) + 1) // macro này tính toán địa chỉ trang kế tiếp bằng cách áp dụng bitwise or với (page_size - 1) để đặt các bit lẻ sau đó thêm 1 để đi đến trang kế tiếp.

//Kiểm tra nếu INITRD_IMAGE_SIZE đã được định nghĩa. Nếu có, gán REAL_INITRD_SIZE bằng INITRD_IMAGE_SIZE. Nếu không, gán REAL_INITRD_SIZE bằng 0. Điều này cho phép mã xử lý tùy chọn kích thước của hình ảnh initrd (Initial RAM Disk).
#ifdef INITRD_IMAGE_SIZE
# define REAL_INITRD_SIZE INITRD_IMAGE_SIZE
#else
# define REAL_INITRD_SIZE 0
#endif

/* Defines from include/asm-alpha/system.h

	BOOT_ADDR	Virtual address at which the consoles loads
			the BOOTP image.

	KERNEL_START    KSEG address at which the kernel is built to run,
			which includes some initial data pages before the
			code.

	START_ADDR	KSEG address of the entry point of kernel code.

	ZERO_PGE	KSEG address of page full of zeroes, but 
			upon entry to kernel, it can be expected
			to hold the parameter list and possible
			INTRD information.

   These are used in the local defines below.
*/
  

/* Virtual addresses for the BOOTP image. Note that this includes the
   bootstrapper code as well as the compressed kernel image, and
   possibly the INITRD image.

   Oh, and do NOT forget the STACK, which appears to be placed virtually
   beyond the end of the loaded image.
*/

#define V_BOOT_IMAGE_START	BOOT_ADDR //đây là một macro định nghĩa V_BOOT_IMAGE_START bằng BOOT_ADDR điều này có nghĩa là bất cứ khi nào V_BOOT_IMAGE_START xuất hiện trong mã, nó sẽ được thay thế bằng giá trị của BOOT_ADDR.
#define V_BOOT_IMAGE_END	SP_on_entry //tương tự, macro này định nghĩa V_BOOT_IMAGE_END bằng SP_on_entry. bất cứ khi nào V_BOOT_IMAGE_END xuất hiện, nó sẽ được thay thế bằng giá trị của SP_on_entry.

/* Virtual addresses for just the bootstrapper part of the BOOTP image. */
//định nghĩa các địa chỉ trong một kernel hoặc bootloader có tác dụng quan trọng trong quá trình khởi động
#define V_BOOTSTRAPPER_START	BOOT_ADDR // định nghĩa điểm bắt đầu cho quá trình bootstrapping của kernel. BOOT_ADDR thường được đặt thành một địa chỉ cụ thể trong bộ nhớ, nơi mà bootloader (như GRUB hoặc một bootloader tùy chỉnh) tải kernel vào bộ nhớ. việc định nghĩa này giúp xác định rõ ràng vùng bộ nhớ mà quá trình khởi động sẽ bắt đầu. khi kernel được nạp, nó sẽ bắt đầu thực thi từ địa chỉ này. điều này là rất quan trọng để đảm bảo rằng kernel khởi động đúng cách và có thể truy cập vào các tài nguyên hệ thống cần thiết.
#define V_BOOTSTRAPPER_END	KERNEL_ORIGIN // định nghĩa điểm kết thúc cho quá trình bootstrapping. KERNEL_ORIGIN thường là địa chỉ mà kernel thực sự bắt đầu thực thi sau khi bootloader đã hoàn tất công việc của mình. địa chỉ này cũng có thể được cấu hình để phù hợp với các thông số khác nhau của kernel. macro này giúp xác định giới hạn cho quá trình khởi động, từ đó cho phép kernel có thể tiếp nhận điều khiển từ bootloader và bắt đầu khởi động các dịch vụ và tiến trình cần thiết.
//hai macro này giúp quản lý các địa chỉ trong bộ nhớ liên quan đến quá trình khởi động kernel. chúng đóng vai trò quan trọng trong việc xác định vị trí bắt đầu và kết thúc cho quá trình khởi động, đảm bảo rằng kernel được nạp và thực thi đúng cách.

/* Virtual addresses for just the data part of the BOOTP
   image. This may also include the INITRD image, but always
   includes the STACK.
*/
//các macro này liên quan đến việc quản lý bộ nhớ trong hạt nhân linux, đặc biệt là liên quan đến vị trí và kích thước của kernel và initrd (initial RAM disk)
#define V_DATA_START		KERNEL_ORIGIN //định nghĩa điểm bắt đầu của dữ liệu kernel, KERNEL_ORIGIN thường là địa chỉ mà kernel bắt đầu được nạp vào bộ nhớ. với macro này, dữ liệu mà kernel sử dụng sẽ bắt đầu từ vị trí này. điều này giúp xác định nơi mà các biến và cấu trúc dữ liệu của kernel sẽ được đặt trong bộ nhớ.
#define V_INITRD_START		(KERNEL_ORIGIN + KERNEL_Z_SIZE) // định nghĩa điểm bắt đầu của initrd. KERNEL_Z_SIZE là kích thước của kernel đã nén. khi cộng với KERNEL_ORIGIN, ta có được địa chỉ bắt đầu của initrd, nơi mà nó sẽ được nạp vào bộ nhớ. điều này giúp hệ thống biết vị trí mà initrd bắt đầu. initrd thường được sử dụng để chứa các driver và hệ thống file cần thiết để khởi động trước khi chuyển điều khiển cho hệ thống file thực sự.
#define V_INTRD_END		(V_INITRD_START + REAL_INITRD_SIZE) // định nghĩa điểm kết thúc của initrd. REAL_INITRD_SIZE là kích thước thực tế của initrd. cộng nó với V_INITRD_START để xác định địa chỉ kết thúc của initrd. điều này xác định ranh giới cho vùng nhớ mà initrd sử dụng, giúp tránh xung đột với các vùng nhớ khác và cho phép kernel biết khi nào initrd kết thúc.
#define V_DATA_END	 	V_BOOT_IMAGE_END //định nghĩa điểm kết thúc của dữ liệu kernel. V_BOOT_IMAGE_END là địa chỉ kết thúc của hình ảnh boot (boot image), có thể bao gồm cả kernel và initrd. điều này giúp xác định ranh giới cho dữ liệu mà kernel sử dụng, từ đó giúp quản lý bộ nhớ tốt hơn và tránh xung đột.
//Các macro này giúp quản lý bộ nhớ cho kernel và initrd trong hạt nhân Linux, xác định rõ các vị trí bắt đầu và kết thúc của từng phần trong bộ nhớ.
// Việc xác định chính xác các địa chỉ này là rất quan trọng để đảm bảo kernel và các thành phần khởi động khác có thể hoạt động một cách hiệu quả và không xung đột với nhau.

/* KSEG addresses for the uncompressed kernel.

   Note that the end address includes workspace for the decompression.
   Note also that the DATA_START address is ZERO_PGE, to which we write
   just before jumping to the kernel image at START_ADDR.
 */
//các macro này liên quan đến việc xác định các địa chỉ trong bộ nhớ cho dữ liệu kernel và hình ảnh kernel trong hạt nhân linux.
#define K_KERNEL_DATA_START	ZERO_PGE //định nghĩa địa chỉ bắt đầu cho dữ liệu của kernel. ZERO_PGE có thể là địa chỉ mà kernel sử dụng để lưu trữ dữ liệu khởi tạo(không được sử dụng) trong bộ nhớ. đây thường là địa chỉ của trang bộ nhớ đầu tiên (zero page). việc khởi đầu dữ liệu kernel tại ZERO_PGE giúp đảm bảo rằng kernel có một khu vực nhớ an toàn để làm việc, đồng thời giảm thiểu rủi ro xung đột với các phần khác trong bộ nhớ.
#define K_KERNEL_IMAGE_START	START_ADDR //định nghĩa địa chỉ bắt đầu cho hình ảnh kernel. START_ADDR thường là địa chỉ mà kernel được nạp vào bộ nhớ khi khởi động. đây là vị trí mà bộ khởi động (bootloader tải kernel). macro này giúp xác định vị trí mà kernel bắt đầu thực thi. điều này rất quan trọng để kernel có thể khởi động và thực hiện các tác vụ cần thiết ngay lập tức.
#define K_KERNEL_IMAGE_END	(START_ADDR + KERNEL_SIZE) //định nghĩa địa chỉ kết thúc cho hình ảnh kernel. KERNEL_SIZE là kích thước của kernel. cộng nó với START_ADDR cho phép định địa chỉ kết thúc của hình ảnh kernel. điều này giúp xác định ranh giới của hình ảnh kernel trong bộ nhớ. việc biết địa chỉ kết thúc là rất quan trọng để tránh xung đột với các vùng bộ nhớ khác và để xác định các khu vực nhớ mà kernel có thể sử dụng sau này.
//Các macro này giúp xác định vị trí trong bộ nhớ cho dữ liệu kernel và hình ảnh kernel trong hạt nhân Linux.
// Việc quản lý các địa chỉ này rất quan trọng để đảm bảo rằng kernel khởi động một cách hiệu quả và không xung đột với các thành phần khác trong bộ nhớ.

/* Define to where we may have to decompress the kernel image, before
   we move it to the final position, in case of overlap. This will be
   above the final position of the kernel.

   Regardless of overlap, we move the INITRD image to the end of this
   copy area, because there needs to be a buffer area after the kernel
   for "bootmem" anyway.
*/
//các macro này liên quan đến việc quản lý bộ nhớ trong hạt nhân linux, đặc biệt là trong quá trình thiết lập và sử dụng bộ nhớ cho kernel và initrd (initial ram disk)
#define K_COPY_IMAGE_START	NEXT_PAGE(K_KERNEL_IMAGE_END) //định nghĩa địa chỉ bắt đầu cho việc sao chép hình ảnh kernel. NEXT_PAGE là một macro (hoặc hàm) có thể được sử dụng để lấy địa chỉ của trang bộ nhớ tiếp theo. khi sử dụng K_KERNEL_IMAGE_END, nó xác định điểm bắt đầu cho việc sao chép hình ảnh kernel ngay sau khi hình ảnh kernel đã nạp vào bộ nhớ. việc này giúp phân tách rõ ràng giữa hình ảnh kernel và các phần khác của bộ nhớ, tránh xung đột và cho phép sử dụng các vùng bộ nhớ khác cho các tác vụ khác.
/* Reserve one page below INITRD for the new stack. */
#define K_INITRD_START \
    NEXT_PAGE(K_COPY_IMAGE_START + KERNEL_SIZE + PAGE_SIZE) //định nghĩa địa chỉ bắt đầu cho initrd. K_COPY_IMAGE_START + KERNEL_SIZE + PAGE_SIZE xác định vị trí bắt đầu cho initrd, thêm một trang bộ nhớ nữa để đảm bảo rằng có đủ không gian cho initrd mà không gây ra xung đột với các phần khác. điều này giúp đảm bảo rằng initrd được nạp vào một vị trí an toàn trong bộ nhớ, cho phép nó hoạt động mà không xung đột với hình ảnh kernel hoặc các phần khác của hệ thống.
#define K_COPY_IMAGE_END \
    (K_INITRD_START + REAL_INITRD_SIZE + MALLOC_AREA_SIZE) //định nghĩa địa chỉ kết thúc cho việc sao chép hình ảnh kernel. K_INITRD_START + REAL_INITRD_SIZE + MALLOC_AREA_SIZE tính toán địa chỉ kết thúc của vùng nhớ được sử dụng để sao chép hình ảnh kernel, cộng thêm kích thước của initrd và khu vực cấp phát bộ nhớ (malloc area). điều này giúp xác định ranh giới cho vùng bộ nhớ mà kernel sử dụng để sao chép, cho phép kernel quản lý bộ nhớ hiệu quả và tránh xung đột.
#define K_COPY_IMAGE_SIZE \
    NEXT_PAGE(K_COPY_IMAGE_END - K_COPY_IMAGE_START) // định nghĩa kích thước của vùng bộ nhớ được sử dụng để sao chép hình ảnh kernel. kích thước được tính bằng cách lấy địa chỉ kết thúc của việc sao chép hình ảnh kernel trừ đi địa chỉ bắt đầu. và sua đó làm tròn lên trang tiếp theo. việc này rất quan trọng để quản lý hiệu quả bộ nhớ và đảm bảo rằng không có phần nào trong bộ nhớ bị lãng phí, đồng thời giúp xác định kích thước chính xác của vùng bộ nhớ cần thiết cho các tác vụ của kernel.
	//Các macro này giúp quản lý vị trí và kích thước của các vùng bộ nhớ liên quan đến việc sao chép hình ảnh kernel và khởi động initrd trong hạt nhân Linux.
	// Việc xác định rõ ràng các địa chỉ và kích thước này rất quan trọng để đảm bảo rằng kernel hoạt động một cách hiệu quả và không gây ra xung đột bộ nhớ.

//đoạn mã này là hàm start_kernel trong hạt nhân linux, có nhiệm vụ khởi động kernel. đây là một phần rất quan trọng trong quá trình khởi động của hệ điều hành
void start_kernel(void){
	//khai báo biến. khia báo các biến để theo dõi trạng thái của hình ảnh kernel chưa giải nén, hình ảnh kernel đã giải nén và vị trí bắt đầu của initrd.
	int must_move = 0;

	/* Initialize these for the decompression-in-place situation,
	   which is the smallest amount of work and most likely to
	   occur when using the normal START_ADDR of the kernel
	   (currently set to 16MB, to clear all console code.
	*/
	unsigned long uncompressed_image_start = K_KERNEL_IMAGE_START;
	unsigned long uncompressed_image_end = K_KERNEL_IMAGE_END;

	unsigned long initrd_image_start = K_INITRD_START;

	/*
	 * Note that this crufty stuff with static and envval
	 * and envbuf is because:
	 *
	 * 1. Frequently, the stack is short, and we don't want to overrun;
	 * 2. Frequently the stack is where we are going to copy the kernel to;
	 * 3. A certain SRM console required the GET_ENV output to stack.
	 *    ??? A comment in the aboot sources indicates that the GET_ENV
	 *    destination must be quadword aligned.  Might this explain the
	 *    behaviour, rather than requiring output to the stack, which
	 *    seems rather far-fetched.
	 */

	//biến tĩnh và thông số. nbytes dùng để lưu trữ kích thước của biến môi trường. envval là mảng để lưu giá trị biến môi trường. asm_sp lấy giá trị stack pointer hiện tại.
	static long nbytes;
	static char envval[256] __attribute__((aligned(8)));
	register unsigned long asm_sp asm("30");

	SP_on_entry = asm_sp;
	//in thông báo khởi động. in ra thông báo khởi động của kernel
	srm_printk("Linux/Alpha BOOTPZ Loader for Linux " UTS_RELEASE "\n");

	/* Validity check the HWRPB. */
	//kiểm tra các thông số của phần cứng. kiểm tra kích thước trang và địa chỉ vptb (virtual page table base). nếu không đúng, kernel sẽ in ra thông báo và thoát.
	if (INIT_HWRPB->pagesize != 8192) {
		srm_printk("Expected 8kB pages, got %ldkB\n",
		           INIT_HWRPB->pagesize >> 10);
		return;
	}
	if (INIT_HWRPB->vptb != (unsigned long) VPTB) {
		srm_printk("Expected vptb at %p, got %p\n",
			   VPTB, (void *)INIT_HWRPB->vptb);
		return;
	}

	/* PALcode (re)initialization. */
	//khởi tạo pal. gọi hàm khởi tạo PAL(platform abstraction layer), chuẩn bị cho các thao tác với phần cứng
	pal_init();

	/* Get the parameter list from the console environment variable. */
	//lây biến môi trường, lấy giá trị của biến môi trường ENV_BOOTED_OSFLAGS và lưu vào envval. nếu không lấy được, thiết lập kích thước là 0.
	nbytes = callback_getenv(ENV_BOOTED_OSFLAGS, envval, sizeof(envval));
	if (nbytes < 0 || nbytes >= sizeof(envval)) {
		nbytes = 0;
	}
	envval[nbytes] = '\0';

#ifdef DEBUG_ADDRESSES
	srm_printk("START_ADDR 0x%lx\n", START_ADDR);
	srm_printk("KERNEL_ORIGIN 0x%lx\n", KERNEL_ORIGIN);
	srm_printk("KERNEL_SIZE 0x%x\n", KERNEL_SIZE);
	srm_printk("KERNEL_Z_SIZE 0x%x\n", KERNEL_Z_SIZE);
#endif

	/* Since all the SRM consoles load the BOOTP image at virtual
	 * 0x20000000, we have to ensure that the physical memory
	 * pages occupied by that image do NOT overlap the physical
	 * address range where the kernel wants to be run.  This
	 * causes real problems when attempting to cdecompress the
	 * former into the latter... :-(
	 *
	 * So, we may have to decompress/move the kernel/INITRD image
	 * virtual-to-physical someplace else first before moving
	 * kernel /INITRD to their final resting places... ;-}
	 *
	 * Sigh...
	 */

	/* First, check to see if the range of addresses occupied by
	   the bootstrapper part of the BOOTP image include any of the
	   physical pages into which the kernel will be placed for
	   execution.

	   We only need check on the final kernel image range, since we
	   will put the INITRD someplace that we can be sure is not
	   in conflict.
	 */
	//kiểm tra xung đột vùng nhớ, kiểm tra xem có xung đột giữa vùng nhớ của bootstrapper và vùng dữ liệu của kernel hay không. nếu có, in ra thông báo lỗi và dừng.
	if (check_range(V_BOOTSTRAPPER_START, V_BOOTSTRAPPER_END,
			K_KERNEL_DATA_START, K_KERNEL_IMAGE_END))
	{
		srm_printk("FATAL ERROR: overlap of bootstrapper code\n");
		__halt();
	}

	/* Next, check to see if the range of addresses occupied by
	   the compressed kernel/INITRD/stack portion of the BOOTP
	   image include any of the physical pages into which the
	   decompressed kernel or the INITRD will be placed for
	   execution.
	 */
	if (check_range(V_DATA_START, V_DATA_END,
			K_KERNEL_IMAGE_START, K_COPY_IMAGE_END))
	{
#ifdef DEBUG_ADDRESSES
		srm_printk("OVERLAP: cannot decompress in place\n");
#endif
		uncompressed_image_start = K_COPY_IMAGE_START;
		uncompressed_image_end = K_COPY_IMAGE_END;
		must_move = 1;

		/* Finally, check to see if the range of addresses
		   occupied by the compressed kernel/INITRD part of
		   the BOOTP image include any of the physical pages
		   into which that part is to be copied for
		   decompression.
		*/
		while (check_range(V_DATA_START, V_DATA_END,
				   uncompressed_image_start,
				   uncompressed_image_end))
		{
#if 0
			uncompressed_image_start += K_COPY_IMAGE_SIZE;
			uncompressed_image_end += K_COPY_IMAGE_SIZE;
			initrd_image_start += K_COPY_IMAGE_SIZE;
#else
			/* Keep as close as possible to end of BOOTP image. */
			uncompressed_image_start += PAGE_SIZE;
			uncompressed_image_end += PAGE_SIZE;
			initrd_image_start += PAGE_SIZE;
#endif
		}
	}

	srm_printk("Starting to load the kernel with args '%s'\n", envval);

#ifdef DEBUG_ADDRESSES
	srm_printk("Decompressing the kernel...\n"
		   "...from 0x%lx to 0x%lx size 0x%x\n",
		   V_DATA_START,
		   uncompressed_image_start,
		   KERNEL_SIZE);
#endif
		//giải nén kernel, gọi hàm để giải nén hình ảnh kernel từ địa chỉ uncompressed_image_start vào V_DATA_START.
        decompress_kernel((void *)uncompressed_image_start,
			  (void *)V_DATA_START,
			  KERNEL_SIZE, KERNEL_Z_SIZE);

	/*
	 * Now, move things to their final positions, if/as required.
	 */

#ifdef INITRD_IMAGE_SIZE

	/* First, we always move the INITRD image, if present. */
#ifdef DEBUG_ADDRESSES
	srm_printk("Moving the INITRD image...\n"
		   " from 0x%lx to 0x%lx size 0x%x\n",
		   V_INITRD_START,
		   initrd_image_start,
		   INITRD_IMAGE_SIZE);
#endif
//di chuyển initrd, nếu initrd có kích thước, di chuyển nó từ vị trí đã nạp tới vị trí cuối cùng đã được xác định.
	memcpy((void *)initrd_image_start, (void *)V_INITRD_START,
	       INITRD_IMAGE_SIZE);

#endif /* INITRD_IMAGE_SIZE */

	/* Next, we may have to move the uncompressed kernel to the
	   final destination.
	 */
	if (must_move) {
#ifdef DEBUG_ADDRESSES
		srm_printk("Moving the uncompressed kernel...\n"
			   "...from 0x%lx to 0x%lx size 0x%x\n",
			   uncompressed_image_start,
			   K_KERNEL_IMAGE_START,
			   (unsigned)KERNEL_SIZE);
#endif
		/*
		 * Move the stack to a safe place to ensure it won't be
		 * overwritten by kernel image.
		 */
		move_stack(initrd_image_start - PAGE_SIZE);
		//xử lý dữ liệu cho kernel, nếu cần thiết, di chuyển kernel đã giải nén vào vị trí cuối cùng đã xác định.
		memcpy((void *)K_KERNEL_IMAGE_START,
		       (void *)uncompressed_image_start, KERNEL_SIZE);
	}
	
	/* Clear the zero page, then move the argument list in. */
#ifdef DEBUG_LAST_STEPS
	srm_printk("Preparing ZERO_PGE...\n");
#endif
//chuẩn bị thông tin cho kernel, xóa vùng nhớ tại địa chỉ ZERO_PGE và sao chép các thông tin biến môi trường vào đó.
	memset((char*)ZERO_PGE, 0, PAGE_SIZE);
	strcpy((char*)ZERO_PGE, envval);

#ifdef INITRD_IMAGE_SIZE

#ifdef DEBUG_LAST_STEPS
	srm_printk("Preparing INITRD info...\n");
#endif
	/* Finally, set the INITRD paramenters for the kernel. */
	((long *)(ZERO_PGE+256))[0] = initrd_image_start;
	((long *)(ZERO_PGE+256))[1] = INITRD_IMAGE_SIZE;

#endif /* INITRD_IMAGE_SIZE */

#ifdef DEBUG_LAST_STEPS
	srm_printk("Doing 'runkernel()'...\n");
#endif
//khởi động kernel. gọi hàm runkernel() để bắt đầu thực thi kernel tại địa chỉ đã xác định.
	runkernel();
}
//Hàm start_kernel là một bước quan trọng trong quá trình khởi động của kernel, chịu trách nhiệm thiết lập môi trường và chuẩn bị cho việc thực thi kernel.
// Nó thực hiện nhiều kiểm tra và di chuyển các thành phần cần thiết để đảm bảo rằng kernel có thể hoạt động hiệu quả và chính xác.

 /* dummy function, should never be called. */
//hàm này là một hàm trong hạt nhân linux, được thiết kế để cấp phát bộ nhớ động cho các cấu trúc dữ liệu kernel. hàm trả về một con trỏ đến kiểu dữ liệu void, có thể được chuyển đổi thành bất kỳ kiểu con trỏ nào khác. tham số: kích thước bộ nhớ cần cấp phát, tham số thứ hai được dùng để chỉ định các cờ cấp phát bộ nhớ, cho biết cách thức cấp phát, nhơ có thể tạm dừng hay không, có phải là yều cầu gấp hay không.
void *__kmalloc(size_t size, gfp_t flags){
	//Hàm hiện tại không thực hiện bất kỳ chức năng cấp phát bộ nhớ nào và luôn trả về NULL, có nghĩa là không có bộ nhớ nào được cấp phát.
	return (void *)NULL;
}
//Hàm __kmalloc là một phần quan trọng của hệ thống cấp phát bộ nhớ trong hạt nhân Linux. Tuy nhiên, trong đoạn mã hiện tại, nó không thực hiện bất kỳ chức năng nào. Đoạn mã này có thể được sử dụng trong quá trình phát triển hoặc thử nghiệm và cần được hoàn thiện để thực sự cấp phát bộ nhớ