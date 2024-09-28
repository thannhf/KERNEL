/* đoạn mã này là một script liên kết (linker script) được viết để chỉ định cách thức sắp xếp các phân đoạn bộ nhớ (sections) trong một chương trình ELF (executable and Linkable Format) khi liên kết (linking) đối với kiến trúc Alpha (elf64-alpha). đây là một kiến trúc RISC 64-bit. */


/* SPDX-License-Identifier: GPL-2.0 */ /*chỉ ra rằng tệp này được cấp phép dưới giấy phép công cộng GNU phiên bản 2.0 (GPL-2.0)*/
OUTPUT_FORMAT("elf64-alpha") /*xác định định dạng đầu ra của tệp liên kết là ELP 64-bit cho kiến trúc Alpha*/
ENTRY(__start) /*xác định điểm bắt đầu của chương trình (entry point) là ký hiệu __start. khi chương trình chạy, con trỏ thực thi sẽ bắt đầu từ địa chỉ này*/
printk = srm_printk; /* đây là lệnh gán printk với srm_printk. printk thường được dùng để in các thông báo trong nhân linux, và ở đây nó được liên kết với hàm in của srm (system reference manual) để tương thích với hệ thống Alpha */
SECTIONS
{
  . = 0x20000000; /*đặt địa chỉ bắt đầu của chương trình tại địa chỉ 0x20000000. đây là địa chỉ cơ sở (base address) cho chương trình*/
  .text : { *(.text) } /*đây là phân đoạn chứa mã thực thi của chương trình. tất cả các đoạn mã trong các tệp nguồn có phần .text sẽ được đặt vào đây*/
  _etext = .; /* đặt giá trị của ký hiệu _etext tại địa chỉ hiện tại, tức là ngay sau khi đoạn .text kết thúc */
  PROVIDE (etext = .); /* cung cấp (provide) một ký hiệu thay thế etext cho _etext */
  .rodata : { *(.rodata) *(.rodata.*) } /*phân đoạn này chứa dữ liệu chỉ đọc, như chuỗi ký tự hằng (const char*) hoặc các hằng số (const variables), bao gồm các phần .rodata và tất cả các phần có dạng .rodata.**/
  .data : { *(.data) CONSTRUCTORS } /*đây là phân đoạn chứa các biến toàn cục đã được khởi tạo của chương trình. nó bao gồm các dữ liệu sẽ được ghi khi chương trình chạy, CONSTRUCTORS đặc biệt là trong c++, đây là nơi chứa các hàm khởi tạo sẽ được gọi trước khi main chạy*/
  .got : { *(.got) } /*phân đoạn global offset table (GOT), được sử dụng trong cơ chế địa chỉ động khi chương trình thực hiện liên kết động (dynamic linking)*/
  .sdata : { *(.sdata) } /*phân đoạn dữ liệu tĩnh nhỏ (small data), chứa các biến toàn cục có kích thước nhỏ và nhanh chóng truy xuất*/
  /*ghi nhận địa chỉ hiện tại là _edata ngay sau phần .data kết thúc, đồng thời cung cấp ký hiệu thay thế edata*/
  _edata = .; 
  PROVIDE (edata = .);
  .sbss : { *(.sbss) *(.scommon) }/*phân đoạn chứa các biến toàn cục không khởi tạo kích thước nhỏ small bss*/
  .bss : { *(.bss) *(COMMON) } /*phân đoạn này chứa các biến toàn cục không khởi tạo bss, tức là các biến sẽ được khởi tạo mặc định với giá trị 0 khi chương trình bắt đầu chạy, các biến common trong chương trình c cũng được đặt trong phân đoạn .bss*/
  /*ghi nhận địa chỉ hiện tại sau phân đoạn .bss là _end, đồng thời cung cấp ký hiệu end*/
  _end = . ;
  PROVIDE (end = .);

  .mdebug 0 : { *(.mdebug) }/*có thể chứa thông tin gỡ lỗi*/
  .note 0 : { *(.note) }/*có thể chứa các ghi chú tùy chọn, ví dụ như thông tin về phiên bản của tệp ELF*/
  .comment 0 : { *(.comment) } /*chứa các chuỗi nhận xét có thể được sử dụng bởi các công cụ biên dịch hoặc liên kết*/
}

/*Đây là một linker script định nghĩa cách thức các phân đoạn mã và dữ liệu được sắp xếp trong bộ nhớ khi liên kết tệp ELF cho kiến trúc Alpha. Nó quản lý các ký hiệu như etext, edata, và end để đánh dấu các điểm kết thúc của các phân đoạn tương ứng, đồng thời hỗ trợ các đặc tính của ELF và OMAGIC.*/