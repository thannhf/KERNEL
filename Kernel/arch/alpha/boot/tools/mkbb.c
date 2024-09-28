// SPDX-License-Identifier: GPL-2.0
/* This utility makes a bootblock suitable for the SRM console/miniloader */

/* Usage:
 *	mkbb <device> <lxboot>
 *
 * Where <device> is the name of the device to install the bootblock on,
 * and <lxboot> is the name of a bootblock to merge in.  This bootblock
 * contains the offset and size of the bootloader.  It must be exactly
 * 512 bytes long.
 */

// Cung cấp các hàm để điều khiển tệp (file control). Được sử dụng để thực hiện các thao tác như mở tệp với các quyền đọc, ghi, hoặc chỉ đọc, hay điều chỉnh cách thức truy cập tệp.
#include <fcntl.h>
// Cung cấp các hàm cấp thấp để tương tác với hệ điều hành, như quản lý tệp và I/O, điều khiển quy trình (process control), và cung cấp các hàm liên quan đến thao tác trên các hệ thống giống Unix.
#include <unistd.h>
// Thư viện chứa các hàm quản lý bộ nhớ động, điều khiển quá trình thoát chương trình, cũng như các hàm tiện ích khác như chuyển đổi chuỗi thành số nguyên (atoi()), quản lý ngẫu nhiên, v.v.
#include <stdlib.h>
// Thư viện chuẩn hỗ trợ nhập/xuất tiêu chuẩn như thao tác với bàn phím, màn hình, và tệp. Cung cấp các hàm để in ra màn hình (printf()), đọc dữ liệu (scanf()), và làm việc với tệp (fopen(), fclose()).
#include <stdio.h>

/* Minimal definition of disklabel, so we don't have to include
 * asm/disklabel.h (confuses make)
 */

// Đoạn mã này đảm bảo rằng MAXPARTITIONS sẽ chỉ được định nghĩa một lần trong chương trình với giá trị là 8. Nếu ở nơi nào đó trong chương trình MAXPARTITIONS đã được định nghĩa trước, đoạn mã này sẽ không thay đổi giá trị của nó.
#ifndef MAXPARTITIONS
#define MAXPARTITIONS   8                       /* max. # of partitions */
#endif

//đây còn gọi là đoạn mã tiền xử lý (if not define), đoạn mã này kiểm tra u8 đã được định nghĩa chưa nếu chưa được định nghĩa nó sẽ thực hiện định nghĩa u8 với kiểu char không dấu
#ifndef u8
#define u8 unsigned char
#endif

//đây còn gọi là đoạn mã tiền xử lý (if not define) đoạn mã này kiểm tra u16 đã được định nghĩa chưa nếu chưa được định nghĩa nó sẽ thực hiện việc định nghĩa u16 với kiểu short không dấu
#ifndef u16
#define u16 unsigned short
#endif

// đây còn gọi là đoạn mã tiền xử lý (if not define) đoạn mã này kiểm tra u32 xem đã được định nghĩa hay chưa nếu chưa được định nghĩa nó sẽ thực hiện định nghĩa u32 với kiểu int không dấu
#ifndef u32
#define u32 unsigned int
#endif

// khởi tạo một struct với tên là disklabel là một struct vô danh nên không thể dùng kiểu này ở nơi khác vì nó không có biến
//đoạn mã này định nghĩa một cấu trúc có tên là disklabel mô tả thông tin chi tiết về cấu trúc của một đĩa. đây có thể là một phần của hệ thống quản lý phân vùng hoặc cấu trúc mô tả một ổ đĩa trong hệ điều hành
struct disklabel {
    //biến d_magic có kiểu u32 (unsigned 32-bit integer) và giá trị của nó phải là DISKLABELMAGIC (một hằng số định nghĩa trước). thường là một magic number để xác định sự hợp lệ của nhãn đĩa
    u32	d_magic;				/* must be DISKLABELMAGIC */ // khởi tạo một biến d_magic với kiểu u32
    //hai biến d_type và d_subtype kiểu u16 (unsigned 16-bit integer) có thể mô tả loại và kiểu phụ của ổ đĩa
    u16	d_type, d_subtype; //khởi tạo hai biến d_type và d_subtype chưa được định nghĩa với kiểu u16
    //hai mảng ký tự d_typename và d_packname mỗi mảng có độ dài 16, lưu tên kiểu đĩa và tên gói đĩa
    u8	d_typename[16]; //khởi tạo một mảng d_typename với độ dài 16 chưa được định nghĩa với kiểu u8
    u8	d_packname[16]; //khởi tạo một mảng d_packname với độ dài 16 chưa được định nghĩa với kiểu u8
    // kích thước secto của đĩa (số byte mỗi sector)
    u32	d_secsize; // khởi tạo một biến d_secsize chưa được định nghĩa với kiểu u32
    //số lượng sector trên mỗi stack
    u32	d_nsectors; //khởi tạo một biến d_nsectors chưa được định nghĩa với kiểu u32
    //số lượng track trên mỗi cylinder
    u32	d_ntracks; //khởi tạo một biến d_ntracks chưa được định nghĩa với kiểu u32
    //số lượng cylinder trên ổ đĩa
    u32	d_ncylinders; //khởi tạo một biến d_ncylinders chưa được định nghĩa kiểu dữ liệu là u32
    //số lượng sector trên mỗi cylinder
    u32	d_secpercyl; //khởi tạo một biến d_secpercyl chưa được định nghĩa có kiểu dữ liệu là u32
    // số lượng sector trên mỗi đơn vị ổ đĩa
    u32	d_secprtunit; //khởi tạo một biến d_secprtunit chưa được định nghĩa có kiểu dữ liệu là u32
    // số sector dự phòng trên mỗi track
    u16	d_sparespertrack; //khởi tạo một biến d_sparespertrack chưa được định nghĩa có kiểu dữ liệu là u16
    //số secctor dự phòng trên mỗi cylinder
    u16	d_sparespercyl; //khởi tạo một biến d_sparespercyl chưa được định nghĩa có kiểu dữ liệu là u16
    // số cylinder dự phòng
    u32	d_acylinders; //khởi tạo một biến d_acylinders chưa được định nghĩa có kiểu dữ liệu là u32
    //các tham số về tốc độ quay của ddiaxm interleave (xen kẽ), độ lệch giữa các track, độ lệch giữa các cylinder
    u16	d_rpm, d_interleave, d_trackskew, d_cylskew; //khởi tạo các biến d_rpm, d_innterleave, d_trackskew, d_cylskew chưa được định nghĩa có kiểu dữ liệu là u16
    //thời gian chuyển đầu đọc, thời gian tìm kiếm track, và các cờ trạng thái của ổ đĩa
    u32	d_headswitch, d_trkseek, d_flags; //khởi tạo các biến d_headswitch, d_trkseek, d_flags chưa được định  nghĩa có kiểu dữ liệu là u32
    //mảng chứa dữ liệu ổ đĩa với 5 phần tử
    u32	d_drivedata[5]; //khởi tạo mảng d_drivedata có độ dài là 5 chưa được định nghĩa với kiểu dữ liệu là u32
    //mảng dự phòng với 5 phần tử
    u32	d_spare[5]; //khởi tạo một mảng d_spare có độ dài là 5 chưa được định nghĩa với kiểu dữ liệu là 32
    //biến này giống như d_magic thường được dùng để xác nhận tính toàn vẹn của nhãn đĩa
    u32	d_magic2;				/* must be DISKLABELMAGIC */ //khởi tạo một biến d_magic2 chưa được định nghĩa với kiểu dữ liệu u32
    //biến lưu trữ giá trị checksum để kiểm tra tính toàn vẹn của dữ liệu nhãn đĩa
    u16	d_checksum; //khởi tạo một biến d_checksum chưa được định nghĩa với kiểu dữ liệu là u16
    // số lượng phân vùng trên ổ đĩa
    u16	d_npartitions; // khởi tạo một biến d_npartitions chưa được định nghĩa với kiểu dữ liệu là u16
    //kích thước của boot block (d_bbsize) và superblock (d_sbsize)
    u32	d_bbsize, d_sbsize; //khởi tạo các biến d_bbsize và d_sbsize chưa được định nghĩa với kiểu u32
    //cấu trúc lồng định nghĩa thông tin cho từng phân vùng trên đĩa, với các thành phần sau:
        struct d_partition { // khởi tạo một struct lồng có tên là d_partition có có danh tính nên có thể dùng struct này ở nơi khác 
            //kích thước của phân vùng
            u32	p_size; //khởi tạo một biến p_size chưa được định nghĩa với kiểu u32
            //độ lệch offset của phân vùng trên đĩa
            u32	p_offset; //khởi tạo một biến p_offset chưa được định nghĩa với kiểu u32
            //kích thước của file system trên phân vùng
            u32	p_fsize; //khởi tạo một biến p_fsize chưa được định nghĩa với kiểu u32
            //kiểu hệ thống tập tin của phân vùng
            u8	p_fstype; //khởi tạo một biến p_fstype chưa được định nghĩa với kiểu u8
            //kích thước các khối phân mảnh
            u8	p_frag; //khởi tạo một biến p_frag chưa được định nghĩa với kiểu u8
            //số nhóm cylinder trên phân vùng
            u16	p_cpg; //khởi tạo một biến p_cpg chưa được định nghĩa với kiểu u16
        } d_partitions[MAXPARTITIONS]; //định nghĩa đây là một mảng kiểu struct d_partition với một mảng có kích thước được xác định bởi hằng số MAXPARTITIONS đoạn mã này dùng để định nghĩa một cấu trúc lồng cho phân vùng trên đĩa giúp tổ chức và lưu trữ thông tin liên quan đến phân vùng một cách có hệ thống việc sử dụng mảng này cho phép dễ dàng quản lý nhiều phân vùng cùng lúc
};

// đoạn typedef này định nghĩa một kiểu dữ liệu union có tên là bootblock, thường được sử dụng để đại diện cho một "boot block" trong hệ thống có thể ở mức hệ thống thấp như ổ đĩa hoặc bộ nạp khởi động 
//định nghĩa union: một union trong c cho phép nhiều thành phần chia sẻ cùng một không gian bộ nhớ với kích thước của union được xác định bởi thành viên lớn nhất của nó. trong trường hợp này, bootblock có bốn thành viên và nó sẽ chiếm kích thước của thành viên lớn nhất cụ thể là bootblock_bytes[512] hoặc bootblock_quadwords[64], cả hai đều chiếm 512 byte
typedef union __bootblock {
    // __u1: một cấu trúc chứa
    struct {
        char __pad1[64]; // một mảng 64 phần tử kiểu char (hay byte), dùng để đệm (padding)
        struct disklabel __label; //__label một cấu trúc struct disklabel, chưa được định nghĩa ở đây, nhưng có thể chứa thông tin về cấu trúc của đĩa
    } __u1;
    //__u2: một cấu trúc chứa
    struct {
	    unsigned long __pad2[63]; //một mảng 63 phần tử kiểu unsigned long, dùng để đệm
	    unsigned long __checksum; // Một giá trị unsigned long lưu trữ giá trị checksum.
    } __u2;
    char bootblock_bytes[512]; //Một mảng 512 phần tử kiểu char, đại diện cho toàn bộ boot block dưới dạng các byte.
    unsigned long bootblock_quadwords[64]; // Một mảng 64 phần tử kiểu unsigned long, đại diện cho boot block dưới dạng 64 quadword (mỗi quadword là 8 byte), với tổng kích thước của thành phần này cũng là 512 byte (64 x 8 byte).
} bootblock; //Nói chung, mục đích của union này là cho phép truy cập boot block dưới các định dạng khác nhau như byte, quadword, hoặc thông qua các cấu trúc cụ thể.
// hai dòng mã này định nghĩa các macro sử dụng để truy cập các trường trong cấu trúc bootblock thông qua các tên ngắn gọn hơn
//macro này định nghĩa bootblock_label làm alias bí danh cho trường __label bên trong cấu trúc con __u1 của union bootblock
//như vậy thay vì phải viết __u1.__label để truy cập nhãn đĩa (disk label) trong bootblock, bạn chỉ cần viết bootblock_label
#define	bootblock_label	__u1.__label
//Macro này định nghĩa bootblock_checksum làm alias cho trường __checksum bên trong cấu trúc con __u2 của union bootblock.
//Điều này giúp đơn giản hóa việc truy cập trường __checksum bằng cách viết bootblock_checksum thay vì __u2.__checksum.
#define bootblock_checksum __u2.__checksum
//đoạn mã này là một chương trình c để thực hiện việc sao chép nhãn đĩa disklabel từ một thiết bị (ví dụ ổ đĩa) sang một hình ảnh bộ nạp khởi động (bootloader image) và tính toán checksum cho bộ nạp khởi động trước khi ghi nó lại vào thiết bị
int main(int argc, char ** argv){
    //khai báo một biến kiểu bootblock để lưu boot block hiện tại từ thiết bị đĩa
    bootblock bootblock_from_disk;
    //khai báo một biến kiểu bootblock để lưu hình ảnh bộ nạp khởi động
    bootblock bootloader_image;
    //các biến để lưu mô tả file cho thiết bị (dev) và file bộ nạp khởi động fd
    int	dev, fd;
    //biến i để lặp qua các phần tử trong quá trình tính checksum và nread để kiểm tra số byte đã đọc
    int	i;
    int	nread;
    
    if(argc != 3) {
	    fprintf(stderr, "Usage: %s device lxboot\n", argv[0]);
	    exit(0);
    }

    /* First, open the device and make sure it's accessible */
    // Thiết bị đích (được cung cấp qua argv[1]) được mở với chế độ đọc và ghi (O_RDWR).
    // Hình ảnh bộ nạp khởi động (được cung cấp qua argv[2]) được mở ở chế độ chỉ đọc (O_RDONLY).
    // Nếu không mở được file hoặc thiết bị, chương trình sẽ in thông báo lỗi và thoát.
    // Đọc dữ liệu từ file bộ nạp khởi động và thiết bị:
    dev = open(argv[1], O_RDWR);
    if(dev < 0) {
        perror(argv[1]);
        exit(0);
    }
    /* Now open the lxboot and make sure it's reasonable */
    fd = open(argv[2], O_RDONLY);
    if(fd < 0) {
        perror(argv[2]);
        close(dev);
        exit(0);
    }

    /* Read in the lxboot */
    // Chương trình đọc boot block từ file bộ nạp khởi động (lxboot) vào biến bootloader_image.
    // Sau đó, chương trình đọc boot block từ thiết bị vào biến bootblock_from_disk.
    // Sao chép nhãn đĩa từ thiết bị vào hình ảnh bộ nạp khởi động:
    nread = read(fd, &bootloader_image, sizeof(bootblock));
    if(nread != sizeof(bootblock)) {
        perror("lxboot read");
        fprintf(stderr, "expected %zd, got %d\n", sizeof(bootblock), nread);
        exit(0);
    }

    /* Read in the bootblock from disk. */
    // Trường bootblock_label từ bootblock_from_disk (boot block từ đĩa) được sao chép vào bootblock_label của bootloader_image (boot block của bộ nạp khởi động).
    nread = read(dev, &bootblock_from_disk, sizeof(bootblock));
    if(nread != sizeof(bootblock)) {
        perror("bootblock read");
        fprintf(stderr, "expected %zd, got %d\n", sizeof(bootblock), nread);
        exit(0);
    }
    /* Swap the bootblock's disklabel into the bootloader */
    bootloader_image.bootblock_label = bootblock_from_disk.bootblock_label;

    /* Calculate the bootblock checksum */
    // Tính toán checksum cho boot block:
    // Trường bootblock_checksum của bootloader_image được khởi tạo bằng 0.
    // Chương trình tính checksum bằng cách cộng giá trị của 63 phần tử trong mảng bootblock_quadwords của bootloader_image.
    // Ghi boot block đã cập nhật trở lại vào thiết bị:
    /* Make sure of the arg count */
    // Chương trình yêu cầu 2 đối số đầu vào: thiết bị đích (device) và hình ảnh bộ nạp khởi động (lxboot).
    // Nếu số lượng tham số không chính xác, chương trình sẽ in thông báo hướng dẫn cách sử dụng và thoát.
    // Mở thiết bị và file bộ nạp khởi động:
    bootloader_image.bootblock_checksum = 0;
    for(i = 0; i < 63; i++) {
	bootloader_image.bootblock_checksum += 
			bootloader_image.bootblock_quadwords[i];
    }

    /* Write the whole thing out! */
    // Sau khi tính checksum, chương trình di chuyển con trỏ file về đầu thiết bị bằng lệnh lseek(dev, 0L, SEEK_SET) và ghi bootloader_image vào thiết bị.
    // Nếu có lỗi xảy ra trong quá trình ghi, chương trình in thông báo lỗi và thoát.
    lseek(dev, 0L, SEEK_SET);
    if(write(dev, &bootloader_image, sizeof(bootblock)) != sizeof(bootblock)) {
	perror("bootblock write");
	exit(0);
    }
    //  // Đóng các file descriptor:

    // Sau khi ghi xong, cả file bộ nạp khởi động (fd) và thiết bị (dev) được đóng lại.
    // Chức năng của chương trình:
    // Chương trình này có nhiệm vụ:

    // Đọc boot block từ thiết bị và từ file bộ nạp khởi động.
    // Sao chép nhãn đĩa từ boot block trên thiết bị vào boot block của bộ nạp khởi động.
    // Tính toán checksum cho bộ nạp khởi động.
    // Ghi boot block mới (đã cập nhật nhãn đĩa và checksum) trở lại thiết bị.
    close(fd);
    close(dev);
    exit(0);
}