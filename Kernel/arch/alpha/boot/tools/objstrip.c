// SPDX-License-Identifier: GPL-2.0
/*
 * arch/alpha/boot/tools/objstrip.c
 *
 * Strip the object file headers/trailers from an executable (ELF or ECOFF).
 *
 * Copyright (C) 1996 David Mosberger-Tang.
 */
/*
 * Converts an ECOFF or ELF object file into a bootable file.  The
 * object file must be a OMAGIC file (i.e., data and bss follow immediately
 * behind the text).  See DEC "Assembly Language Programmer's Guide"
 * documentation for details.  The SRM boot process is documented in
 * the Alpha AXP Architecture Reference Manual, Second Edition by
 * Richard L. Sites and Richard T. Witek.
 */
// Thư viện này cung cấp các hàm tiêu chuẩn để thực hiện các thao tác nhập/xuất (input/output) như printf(), fprintf(), fscanf(), scanf(), và perror().
#include <stdio.h>
// Thư viện này chứa các hàm thao tác chuỗi và bộ nhớ như memcpy(), strcmp(), strlen(), strcpy(), và memset(). Trong trường hợp của chương trình của bạn, bạn có thể sử dụng memcpy() để sao chép dữ liệu giữa các cấu trúc.
#include <string.h>
// Thư viện này cung cấp các hàm quản lý bộ nhớ động và các tiện ích khác như malloc(), calloc(), free(), và exit(). Chương trình của bạn sử dụng exit() để kết thúc chương trình khi xảy ra lỗi.
#include <stdlib.h>
// Thư viện này cung cấp các hàm hệ thống của POSIX như read(), write(), close(), lseek(), và access(). Đây là các hàm được sử dụng để thao tác với file descriptors, thực hiện các hành động đọc, ghi, và di chuyển con trỏ file trong chương trình của bạn.
#include <unistd.h>
//Khi thêm dòng #include <sys/fcntl.h>, bạn đang sử dụng thư viện fcntl.h (File Control), một phần của tiêu chuẩn POSIX. Thư viện này cung cấp các hằng số và khai báo cho các thao tác điều khiển file, như mở file với các chế độ khác nhau. 
#include <sys/fcntl.h>
// Thư viện này cung cấp các khai báo và hàm liên quan đến thuộc tính file và thư mục. Các hàm này cho phép bạn thao tác với quyền truy cập, tạo thư mục, và kiểm tra thuộc tính của file.
#include <sys/stat.h>
//Thư viện này định nghĩa các kiểu dữ liệu cơ bản được sử dụng trong các lời gọi hệ thống liên quan đến file và tiến trình. Nó bao gồm các kiểu dữ liệu như size_t, off_t, pid_t, và mode_t.
#include <sys/types.h>
//thư viện này định nghĩa các cấu trúc và hằng số cho định dạng tệp thực thi a.out (assembler output). đây là một định dạng tệp thực thi cũ được sử dụng trong các hệ thông unix trước khi elf (executable and linkable format) trở nên phổ biến
// thư viện này cung cấp các cấu trúc như struct exec dùng để mô tả phần đầu của một file thực thi a.out 
#include <linux/a.out.h>
// thư viện này cung cấp các khai báo cho định dạng tệp COFF (common Object File Format), một định dạng file thực thi và đối tượng khác, cũng từng được sử dụng trên nhiều hệ điều hành, nhưng đã dần được thay thế bởi ELF. COFF thường được dùng để tổ chức phân đoạn (section) khác nhau của tệp nhị phân chẳng hạn như các phần mã, dữ liệu và biểu tượng
#include <linux/coff.h>
// thư viện này định nghĩa các tham số hệ thống cơ bản (system parameters) cho linux. nó cung cấp các hằng số về hệ thống như kích thước bộ nhớ, số tiến trình tối đa, tốc độ đồng hồ hệ thống.v.v.
#include <linux/param.h>
//đoạn mã liên quan đến việc kiểm tra xem chương trình có đang sử dụng định dạng tệp thực thi ELF không. 
// kiểm tra xem định nghĩa __ELF__ có tồn tại hay không. nếu chương trình được biên dịch với hỗ trợ ELF, hằng số ___ELF__ sẽ được định nghĩa, và đoạn mã bên trong sẽ được biên dịch.
#ifdef __ELF__
	// nếu elf được hỗ trợ, thư viện elf.h sẽ  được bao gồm. thư viện này định nghĩa các cấu trúc và hằng số cho định dạng tệp elf, bao gồm các tiêu đề elf, bảng phân đoạn, và nhiều định nghĩa khác cần thiết để thao tác với tệp elf.
	# include <linux/elf.h>
	// cấu trúc tiêu đề elf 64-bit, chứa thông tin về định dạng và cách hệ điều hành tải tệp vào bộ nhớ
	// dòng này định nghĩa elfhdr là bí danh cho elf64_hdr, sử dụng cấu trúc elf cho định dạng 64-bit. điều này giúp giảm việc phải lặp lại cấu trúc elf64_hdr nhiều lần trong mã
	# define elfhdr elf64_hdr
	// cấu trúc mô tả bảng phân đoạn (program header), chứa thông tin về các đoạn chương trình được nạp vào bộ nhớ
	// elf_phdr được định nghĩa là bí danh cho elf64_phdr, biểu thị bảng phân đoạn trong ELF 64-bit
	# define elf_phdr elf64_phdr
	//dòng này định nghĩa một macro để kiểm tra kiến trúc (architecture) của tệp ELF. trong trường hợp này, nó kiểm tra xem kiến trúc của tệp ELF có phải là Alpha (kiến trúc DEC Alpha) hay không
	// e_machine là một trường trong tiêu đề ELF (cấu trúc elf64_hdr) mô tả loại kiến trúc của file thực thi. EM_ALPHA là một hằng số biểu thị kiến trúc Alpha
	# define elf_check_arch(x) ((x)->e_machine == EM_ALPHA)
#endif

/* bootfile size must be multiple of BLOCK_SIZE: */
// đoạn mã này định nghĩa một hằng số và một hàm để hiển thị cách sử dụng của chương trình
// dòng này định nghĩa một hằng số BLOCK_SIZE với giá trị 512. hằng số này có thể được sử dụng trong chương trình để đại diện cho kích thước của một block dữ liệu thường được dùng trong ngữ cảnh lưu trữ hoặc xử lý tệp.
#define BLOCK_SIZE	512
// biến prog_name được khai báo là một con trỏ đến chuỗi ký tự(string), có thể dùng để lưu tên của chương trình. biến này thường được gán giá trị trước khi gọi hàm usage() để in ra thông báo
const char * prog_name;
// hàm usage() được định nghĩa là static, có nghĩa là nó chỉ có thể được truy cập trong file nguồn mà nó được định nghĩa, không thể được gọi từ các file khác.
// hàm này không nhận tham số nào và trả về kiểu void
static void usage (void){
	// dòng này sử dụng fprintf để in thông báo sử dụng chương trình ra stderr (standard error). điều này có nghĩa là thông báo lỗi sẽ không bị trộn lẫn với đầu ra chính của chương trình.
	// các thông báo sử dụng %s để chèn tên chương trình prog_name vào trong chuỗi
    fprintf(stderr,
	    "usage: %s [-v] -p file primary\n"
	    "       %s [-vb] file [secondary]\n", prog_name, prog_name);
	// sau khi in thông báo, hàm usage() gọi exit(1) cho biết rằng chương trình đã gặp lỗi và sẽ thoát với mã lỗi 1. mã lỗi khác 0 thường biểu thị rằng có một lỗi đã xảy ra.
	exit(1);
}

// Dòng int main(int argc, char *argv[]) xác định hàm chính của chương trình, cho phép chương trình nhận các tham số từ dòng lệnh. Điều này rất hữu ích để làm cho chương trình linh hoạt và có thể nhận dữ liệu từ người dùng khi chạy.
int main (int argc, char *argv[]){
	// size_t: kiểu dữ liệu này thường được sử dụng để đại diện cho kích thước của các đối tượng trong bộ nhớ. nó đảm bảo rằng kích thước là đủ lớn để chứa các giá trị mà nó cần
	// nwritten: số byte đã được ghi vào file hoặc thiết bị
	// tocopy: số byte cần sao chép từ buffer vào file
	// n: được sử dụng để lưu trữ số lượng byte đã đọc hoặc ghi, hoặc để lặp qua các vòng lặp
	// mem_size: kích thước của bộ nhớ hoặc buffer được sử dụng trong chương trình
	// fil_size: kích thước của file mà chương trình đang làm việc
	// pad: giá trị dùng để đệm (padding), có thể được sử dụng để căn chỉnh kích thước dữ liệu
    size_t nwritten, tocopy, n, mem_size, fil_size, pad = 0;
	// fd: mã file descriptor cho file đầu vào
	// ofd: mã file descriptor cho file đầu ra
	// i, j: các biến kiểu int thường dùng làm biến lặp trong vòng lặp for hoặc để theo dõi các chỉ số
	// verbose: biến này có thể được sử dụng để điều khiển mức độ chi tiết của đầu ra với 0 thường biểu thị rằng chế độ chi tiết bị tắt
	// primary: biến này có thể được sử dụng để chỉ định nếu đây là file chính hay không
    int fd, ofd, i, j, verbose = 0, primary = 0;
	// buf[8192]: một buffer có kích thước 8192 byte, được sử dụng để lưu trữ dữ liệu tạm thời khi đọc từ hoặc ghi vào file
	// *inname: con trỏ kiểu char, có thể được sử dụng để lưu trữ tên của file đầu vào
    char buf[8192], *inname;
	// struct exec: cấu trúc này có thể được sử dụng để lưu trữ thông tin về header của file a.out. Đây là định dạng tệp thực thi cũ trong unix
    struct exec * aout;		/* includes file & aout header */
    // long offset: biến kiểu long có thể được sử dụng để theo dõi vị trí trong file khi đọc hoặc ghi dữ liệu.
	long offset;
// khối lệnh với #ifdef __ELF__:
// nếu hệ thống hỗ trợ ELF các biến sau đây sẽ được định nghĩa:
#ifdef __ELF__
	// con trỏ đế cấu trúc tiêu đề ELF, chứa thông tin về tệp ELF.
	struct elfhdr *elf;
	// Con trỏ đến cấu trúc tiêu đề phân đoạn (program header) trong tệp ELF.
	struct elf_phdr *elf_phdr;	/* program header */
	// Biến này có thể lưu trữ địa chỉ điểm vào (entry point) của chương trình ELF, tức là địa chỉ nơi thực thi bắt đầu.
	unsigned long long e_entry;
#endif
	// đoạn mã tiếp tục phần xử lý các tham số dòng lệnh, và thêm một dòng để gán tên chương trình vào biến prog_name
	// dòng này gán tên của chương trình từ argv[0] cho đến prog_name. điều này giúp dễ dàng sử dụng tên chương trình trong các thông báo 
    prog_name = argv[0];
	// vòng lặp kiểm tra các tham số dòng lệnh, bắt đầu từ argv[1], để xem có tham số nào bắt đầu bằng dấu - hay không.
    for (i = 1; i < argc && argv[i][0] == '-'; ++i) {
		// vòng lặp lồng nhau duyệt qua từng ký tự của tham số hiện tại, bắt đầu từ ký tự thứ hai bỏ qua dấu -.
		for (j = 1; argv[i][j]; ++j) {
			// cấu trúc switch sẽ thực hiện các hành động tùy theo ký tự được tìm thấy.
			switch (argv[i][j]) {
			// nếu ký tự là v, biến verbose sẽ bị lật ngược. điều này cho phép người dùng bật hoặc tắt chế độ chi tiết.
			case 'v':
			verbose = ~verbose;
			break;
			// nếu ký tự là b, biến pad sẽ được gán giá trị BLOCK_SIZE(512), chỉ định kích thước đệm
			case 'b':
			pad = BLOCK_SIZE;
			break;
			// nếu ký tự là p, biến primary sẽ được gán giá trị 1, cho biết rằng bootblock sẽ là bootblock chính.
			case 'p':
			primary = 1;		/* make primary bootblock */
			break;
			}
		}
    }
	// đoạn mã tiếp tục cung cấp xử lý tham số đầu vào và mở file để đọc.
	// kiểm tra xem biến i có vượt qua số lượng tham số argc hay không. nếu i lớn hơn hoặc bằng argc, điều này có nghĩa là không còn tham số nào hợp lệ để xử lý
	// nếu điều kiện này đúng, hàm usage() sẽ được gọi để hiển thị hướng dẫn sử dụng chương trình và sau đó chương trình sẽ thoát.
    if (i >= argc) {
	usage();
    }
	// gán tên của file đầu vào từ tham số dòng lệnh vào biến inname, sau đó tăng giá trị của i lên 1.
	// việc tăng i này cho phép chuyển sang tham số tiếp theo trong các lần lặp tiếp theo (nếu có).
    inname = argv[i++];
	// mở file có tên lưu trong inname với chế độ chỉ đọc (O_RDONLY).
	// kết quả trả về là một file descriptor(fd). nếu việc mở file thành công, fd sẽ là một số dương, nếu không thành công, fd sẽ là -1.
    fd = open(inname, O_RDONLY);
	// kiểm tra xem fd có bằng -1 hay không để xác định xem việc mở file có thành công hay không
	// nếu không thành công, hàm perrror("open") sẽ in ra thông báo lỗi liên quan đến việc mở file, sau đó chương trình sẽ thoát với mã lỗi 1, cho biết có sự cố đã xảy ra.
    if (fd == -1) {
	perror("open");
	exit(1);
    }
	// ofd là file descriptor(fd) của file đầu ra.
	// ban đầu ofd được gán là 1, đại diện cho stdout (đầu ra tiêu chuẩn), nghĩa là nếu không có file đầu ra cụ thể nào được cung cấp, chương trình sẽ xuất kết quả ra màn hình
    ofd = 1;
	// điều kiện này kiểm tra xem còn tham số nào trong argv không, tức là có file đầu ra được chỉ định không
    if (i < argc) {
		// nếu có, nó sẽ cố gắng mở file đó với các cờ sau:
		//O_WRONLY mở file chỉ để ghi
		// O_CREAT tạo file nếu file chưa tồn tại
		//O_TRUNC nếu file đã tồn tại, nội dung sẽ bị xóa trước khi ghi mới
		//quyền truy cập 0666 file sẽ có quyền đọc và ghi cho tất cả người dùng owner, group, others
		ofd = open(argv[i++], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		// nếu việc mở file thất bại (trả về -1), chương trình sẽ in thông báo lỗi và thoát với mã lỗi 1
		if (ofd == -1) {
			perror("open");
			exit(1);
		}
    }
	// điều kiện này kiểm tra xem biến primary có bằng 1 không, tức là chương trình đang xử lý để tạo bootblock cho primary bootloader
    if (primary) {
	/* generate bootblock for primary loader */
	// bb[64] mảng unsigned long có 64 phần tử để lưu trữ dữ liệu cho bootblock
	// sum = 0 biến sum sẽ được dùng để tính toán checksum của bootblock.
	unsigned long bb[64], sum = 0;
	// st là một cấu trúc stat, dùng để lưu trữ thông tin về file như kích thước file, quyền truy cập,...
	struct stat st;
	// size là biến để lưu trữ kich thước của file
	off_t size;
	// i là biến chỉ mục, sẽ được sử dụng trong các vòng lặp
	int i;
	// nếu ofd vẫn là 1 (nghĩa là không có file đầu ra được chỉ định), chương trình sẽ hiển thị hướng dẫn sử dụng usage() và thoát, vì việc tạo primary bootblock yêu cầu một file đầu ra cụ thể
	if (ofd == 1) {
	    usage();
	}
	// fstat(fd, &st) lấy thông tin về file đầu vào (được mở với fd) và lưu trữ thông tin đó trong cấu trúc st.
	//nếu fstat thất bại (trả về -1), chương trình sẽ in thông báo lỗi và thoát với mã lỗi 1.
	if (fstat(fd, &st) == -1) {
	    perror("fstat");
	    exit(1);
	}
	// Đoạn mã này tiếp tục xử lý việc tạo bootblock cho primary bootloader, tính toán checksum, và ghi bootblock ra file đầu ra
	// tính toán kích thước của file sao cho nó được làm tròn lên đến bội số của BLOCK_SIZE (512 byte)
	// st.st_size là kích thước thực tế của file, cộng với BLOCK_SIZE - 1 rồi thực hiện phép toán với & ~(BLOCK_SIZE - 1) để đảm bảo kích thước sẽ là bội số của 512
	// đây là cách xử lý kích thước để bootblock phù hợp với kích thước sector của hệ thống
	size = (st.st_size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);
	// memset được sử dụng để đặt tất cả các giá trị trong mảng bb (bootblock) thành 0. mảng bb có kích thước 64 phần tử kiểu unsigned long
	memset(bb, 0, sizeof(bb));
	// sao chép chuỗi "linux srm bootblock" vào mảng bb. đây là một cách để đánh dấu bootblock, chỉ ra rằng nó là bootblock của linux srm (system reference manual) bootloader
	strcpy((char *) bb, "Linux SRM bootblock");
	// gán cho phần tử thứ 60 của mảng bb giá trị là số block trong file đầu vào, được tính bằng kích thước file chia cho kích thước của một block (512 byte).
	bb[60] = size / BLOCK_SIZE;	/* count */
	// gán cho phần tử thứ 61 giá trị 1, đại diện cho sector bắt đầu (sector đầu tiên)
	bb[61] = 1;			/* starting sector # */
	// gán cho phần tử thứ 62 giá trị 0. đây là trường "flags", yêu cầu phải bằng 0
	bb[62] = 0;			/* flags---must be 0 */
	// vòng lặp tính toán checksum bằng cách cộng dồn tất cả các giá trị từ bb[0] đến bb[62] vào biến sum.
	for (i = 0; i < 63; ++i) {
	    sum += bb[i];
	}
	// gán checksum tính toán được vào phần tử thứ 63 của mảng bb.
	bb[63] = sum;
	// ghi toàn bộ mảng bb (bootblock) ra file đầu ra ofd
	// nếu việc ghi không thành công (số byte ghi được khác kích thước của bb), chương trình sẽ in thông báo lỗi và thoát
	if (write(ofd, bb, sizeof(bb)) != sizeof(bb)) {
	    perror("boot-block write");
	    exit(1);
	}
	// in ra giá trị size (kích thước đã được làm tròn của file) lên màn hình.
	printf("%lu\n", size);
	// trả về 0 để báo hiệu chương trình đã kết thúc thành công.
	return 0;
    }

    /* read and inspect exec header: */
	// hệ thống gọi hàm read() để đọc dữ liệu từ file có file descriptor là fd (được mở trước đó) vào bộ nhớ đệm buf
	// sizeof(buf): kích thước của bộ đệm buf, tức là số byte dữ liệu tối đa sẽ được đọc từ file vào bộ nhớ đệm
	// điều kiện kiểm tra xem giá trị trả về từ read() có nhỏ hơn 0 không, tức là có lỗi trong quá trình đọc
	// nếu < 0 thì giá trị trả về từ read() nhỏ hơn 0 biểu thị một lỗi (không phải đọc được dữ liệu).
    if (read(fd, buf, sizeof(buf)) < 0) {
		// nếu có lỗi xảy ra (khi điều kiện read(fd, buf, sizeof(buf)) < 0 đúng), chương trình sẽ gọi perror("read") để in thông báo lỗi liên quan đến thao tác read() và chuỗi "read" sẽ được thêm vào trước thông báo lỗi để chỉ rõ vị trí xảy ra lỗi
	perror("read");
	// sau khi in thông báo lỗi chương trình sẽ kết thúc ngay lập tức với mã thoát 1, cho biết có lỗi xảy ra.
	exit(1);
    }
// Đoạn mã này thực hiện kiểm tra và xử lý cho tệp thực thi định dạng ELF (Executable and Linkable Format)
// đoạn mã này chỉ được biên dịch nếu chương trình đang chạy trên hệ thống có hỗ trợ định dạng ELF (thường được định nghĩa với macro __ELF__).
#ifdef __ELF__
	// gán con trỏ elf kiểu struct elfhdr (header ELF) tới vùng bộ đệm buf. điều này giả định rằng nội dung buf chứa dữ liệu ELF header
    elf = (struct elfhdr *) buf;
	// so sánh các byte nhận dạng ELF (e_ident) trong ELF header với chuỗi magic ELF(ELFMAG). điều này xác minh rằng tệp được đọc thực sự là tệp ELF. EI_MAG0 là chỉ số trong e_ident của ELF magic bytes
	// memcmp sẽ trả về 0 nếu các byte khớp
    if (memcmp(&elf->e_ident[EI_MAG0], ELFMAG, SELFMAG) == 0) {
	// kiểm tra xem tệp ELF có phải là tệp thực thi hay không (ET_EXEC là loại tệp thực thi). nếu không, chương trình sẽ in thông báo lỗi và thoát.
	if (elf->e_type != ET_EXEC) {
	    fprintf(stderr, "%s: %s is not an ELF executable\n",
		    prog_name, inname);
	    exit(1);
	}
	// kiểm tra kiến trúc của ELF bằng cách gọi hàm elf_check_arch(). hàm này kiểm tra trường e_machine của ELF header để đảm bảo tệp ELF phù hợp với kiến trúc máy đang chạy chương trình(ví dụ: nếu đang trên kiến trúc alpha thì e_machine phải là EM_ALPHA)
	// nếu không khớp, in thông báo lỗi và thoát
	if (!elf_check_arch(elf)) {
	    fprintf(stderr, "%s: is not for this processor (e_machine=%d)\n",
		    prog_name, elf->e_machine);
	    exit(1);
	}
	// kiểm tra số lượng program headers (e_phnum) trong ELF. nếu không phải là 1, có thể tệp ELF chưa được liên kết đúng cách với tùy chọn -N. đây có thể là lỗi trong quá trình liên kết linking
	if (elf->e_phnum != 1) {
	    fprintf(stderr,
		    "%s: %d program headers (forgot to link with -N?)\n",
		    prog_name, elf->e_phnum);
	}
	// lưu giá trị e_entry, là địa chỉ vào của chương trình (nơi CPU sẽ bắt đầu thực thi chương trình ELF) từ ELF header.
	e_entry = elf->e_entry;
	// dịch chuyển con trỏ tệp đến vị trí bắt đầu của program header table (e_phoff) trong tệp ELF. đây là nơi chứa các program header
	lseek(fd, elf->e_phoff, SEEK_SET);
	// đọc một program header từ tệp ELF vào buf. program header chứa thông tin về cách nạp và chạy các phần của tệp thực thi ELF.
	if (read(fd, buf, sizeof(*elf_phdr)) != sizeof(*elf_phdr)) {
	    perror("read");
	    exit(1);
	}
	// gán con trỏ elf_phdr kiểu struct elf_phdr trỏ tới vùng dữ liệu của program header vừa đọc.
	elf_phdr = (struct elf_phdr *) buf;
	// lưu giá trị p_offset, là vị trí trong tệp ELF mà segment này bắt đầu
	offset	 = elf_phdr->p_offset;
	// lưu kích thước của đoạn này trong bộ nhớ p_memsz
	mem_size = elf_phdr->p_memsz;
	// lưu kích thước của đoạn này trong tệp p_filesz
	fil_size = elf_phdr->p_filesz;

	/* work around ELF bug: */
	// nếu địa chỉ nạp của segment p_vaddr nhỏ hơn địa chỉ vào e_entry, đoạn mã này điều chỉnh offset, mem_size, và fil_size để bù trừ cho sự khác biệt này.
	if (elf_phdr->p_vaddr < e_entry) {
	    unsigned long delta = e_entry - elf_phdr->p_vaddr;
	    offset   += delta;
	    mem_size -= delta;
	    fil_size -= delta;
	    elf_phdr->p_vaddr += delta;
	}
	// nếu chế độ verbose được bật, chương trình sẽ in ra thông tin chi tiết về đoạn ELF sẽ được nạp, bao gồm phạm vi địa chỉ trong bộ nhớ p_vaddr đến p_vaddr + fil_size và vị trí trong tệp offset.
 	if (verbose) {
	    fprintf(stderr, "%s: extracting %#016lx-%#016lx (at %lx)\n",
		    prog_name, (long) elf_phdr->p_vaddr,
		    elf_phdr->p_vaddr + fil_size, offset);
	}
	//  trong phần kiểm tra định dạng COFF (nếu không phải ELF) thì:
    } else
#endif
    {
	// gán con trỏ aout kiểu struct exec (định nghĩa tệp COFF) trỏ vào vùng bộ đệm buf vùng này chứa header của tệp thực thi COFF
	aout = (struct exec *) buf;
	// kiểm tra nếu tệp COFF có cờ COFF_F_EXEC, tức là tệp đang kiểm tra có phải là tệp thực thi không. nếu không phải, chương trình in thông báo lỗi và thoát
	if (!(aout->fh.f_flags & COFF_F_EXEC)) {
	    fprintf(stderr, "%s: %s is not in executable format\n",
		    prog_name, inname);
	    exit(1);
	}
	// kiểm tra kích thước của phần header tùy chọn f_opthdr. nếu kích thước không khớp với kích thước aout->ah (optional header), chương trình in ra thông báo lỗi và thoát
	if (aout->fh.f_opthdr != sizeof(aout->ah)) {
	    fprintf(stderr, "%s: %s has unexpected optional header size\n",
		    prog_name, inname);
	    exit(1);	
	}
	// kiểm tra xem tệp COFF có phải là định dạng OMAGIC không. OMAGIC là một kiểu tệp thực thi. Nếu không phải, chương trình in ra thông báo lỗi và thoát.
	if (N_MAGIC(*aout) != OMAGIC) {
	    fprintf(stderr, "%s: %s is not an OMAGIC file\n",
		    prog_name, inname);
	    exit(1);
	}
	// xác định vị trí offset của phần mã thực thi text segment trong tệp COFF. đây là vị trí bắt đầu của phần dữ liệu trong tệp
	offset = N_TXTOFF(*aout);
	// tính kích thước của phần tệp (file size), bao gồm cả phần text và phần data.
	fil_size = aout->ah.tsize + aout->ah.dsize;
	// tính tổng kích thước trong bộ nhớ, bao gồm cả phần bss (phần dữ liệu không khởi tạo) sau phần text và data.
	mem_size = fil_size + aout->ah.bsize;
	// nếu verbose được bật, chương trình sẽ in ra thông tin về phạm vi địa chỉ bộ nhớ của phần text.
	if (verbose) {
	    fprintf(stderr, "%s: extracting %#016lx-%#016lx (at %lx)\n",
		    prog_name, aout->ah.text_start,
		    aout->ah.text_start + fil_size, offset);
	}
    }
	// đặt con trỏ tệp tại vị trí offset đã tính toán từ phần header để đọc dữ liệu từ tệp COFF.
    if (lseek(fd, offset, SEEK_SET) != offset) {
	perror("lseek");
	exit(1);
    }
	// chương trình đọc và ghi dữ liệu từ tệp fd (tệp đầu vào) vào bộ đệm buf, sau đó ghi dữ liệu này ra ofd (tệp đầu ra).
    if (verbose) {
	fprintf(stderr, "%s: copying %lu byte from %s\n",
		prog_name, (unsigned long) fil_size, inname);
    }
	// sử dụng vòng lặp để ghi dữ liệu từ bộ đệm ra tệp cho đến khi toàn bộ dữ liệu được sao chép xong.
    tocopy = fil_size;
    while (tocopy > 0) {
	n = tocopy;
	if (n > sizeof(buf)) {
	    n = sizeof(buf);
	}
	tocopy -= n;
	if ((size_t) read(fd, buf, n) != n) {
	    perror("read");
	    exit(1);
	}
	// write và read kiểm tra lỗi: sau mỗi thao tác ghi và đọc, chương trình kiểm tra lỗi để đảm bảo rằng các thao tác hoàn thành thành công.
	do {
	    nwritten = write(ofd, buf, n);
	    if ((ssize_t) nwritten == -1) {
		perror("write");
		exit(1);
	    }
	    n -= nwritten;
	} while (n > 0);
    }
	// nếu có phần bss (không khởi tạo) sau phần text và data, chương trình sẽ điền các byte 0 vào và căn chỉnh đến kích thước pad (nếu được chỉ định bởi tùy chọn -b)
    if (pad) {
	mem_size = ((mem_size + pad - 1) / pad) * pad;
    }

    tocopy = mem_size - fil_size;
    if (tocopy > 0) {
	fprintf(stderr,
		"%s: zero-filling bss and aligning to %lu with %lu bytes\n",
		prog_name, pad, (unsigned long) tocopy);
	// sử dụng memset để điền bộ đệm với giá trị 0, và tiếp tục ghi phần bss này ra tệp cho đến khi phần dữ liệu được điền đầy đủ
	memset(buf, 0x00, sizeof(buf));
	do {
	    n = tocopy;
	    if (n > sizeof(buf)) {
		n = sizeof(buf);
	    }
	    nwritten = write(ofd, buf, n);
	    if ((ssize_t) nwritten == -1) {
		perror("write");
		exit(1);
	    }
	    tocopy -= nwritten;
	} while (tocopy > 0);
    }
    return 0;
}
// Đoạn mã này xử lý cả hai định dạng ELF và COFF, đọc từ tệp thực thi và ghi dữ liệu vào tệp đích.
// Chương trình cũng bao gồm các biện pháp kiểm tra lỗi trong quá trình đọc và ghi dữ liệu.
// Khi bật chế độ verbose, chương trình sẽ cung cấp thông tin chi tiết về các thao tác đang thực hiện (ví dụ: đọc phần nào của tệp, ghi bao nhiêu byte).