/* Author(s): Edmar Caixeta Filho
 * Creates operating system image suitable for placement on a boot disk
 */
/* TODO: Comment on the status of your submission. Largely unimplemented */
#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512				/* floppy sector size in bytes */
#define BOOTLOADER_SIG_OFFSET 0x1fe /* offset for boot loader signature */
// more defines...

/* Reads in an executable file in ELF format*/
Elf32_Phdr *read_exec_file(FILE **execfile, char *filename, Elf32_Ehdr **ehdr)
{
	/*Open ELF file*/
	*execfile = fopen(filename, "rb");

	if (execfile == NULL)
	{
		printf("[ERROR] Something wrong reading ELF file\n");
		exit(1);
	}

	/*Reading ELF file*/
	fread(*ehdr, sizeof(Elf32_Ehdr), 1, *execfile);

	/*If Program Header Offset = 0, Program Header does not exist*/
	/*Program Header only exists for executable files, so it is an optional header*/
	if ((*ehdr)->e_phoff != 0)
	{
		Elf32_Phdr *program_hdr;
		Elf32_Half program_hdr_num;
		Elf32_Half program_hdr_entry_size;
		Elf32_Half program_hdr_size;

		program_hdr_num = (*ehdr)->e_phnum;
		program_hdr_entry_size = (*ehdr)->e_phentsize;

		/*According to ELF Format Docs: [...] the product of e_phentsize and e_phnum gives the table’s size in bytes.*/
		program_hdr_size = program_hdr_num * program_hdr_entry_size;

		/*Memory Allocation for the Program Header, as cited before, the size is given in bytes and sizeof(char) equals 1 byte.*/
		program_hdr = malloc(program_hdr_size * sizeof(char));
		fread(program_hdr, sizeof(char), program_hdr_size, *execfile);

		return program_hdr;
	}

	/*When the ELF file does not represent an executable program, there isn't a Program Header*/
	else
	{
		/*Do something*/
		printf("[ERROR] Program Header not found");
		exit(1);
	}
}

/* Writes the bootblock to the image file */
void write_bootblock(FILE **imagefile, FILE *bootfile, Elf32_Ehdr *boot_header, Elf32_Phdr *boot_phdr)
{
	unsigned char buffer[SECTOR_SIZE] = {0};
	Elf32_Half p_offset, p_filesz;

	p_offset = (boot_phdr)->p_offset;
	p_filesz = (boot_phdr)->p_filesz;

	fseek(bootfile, p_offset, SEEK_SET);
	fread(buffer, p_filesz, 1, bootfile);

	/*MBR Boot signature constants*/
	unsigned char boot_signature_first = 0x55;
	unsigned char boot_signature_last = 0xAA;
	buffer[BOOTLOADER_SIG_OFFSET] = boot_signature_first;
	buffer[BOOTLOADER_SIG_OFFSET + 1] = boot_signature_last;

	/*Write bootloader buffer into image file*/
	fwrite(buffer, sizeof(buffer), 1, *imagefile);
}

/* Counts the number of sectors in the kernel */
int count_kernel_sectors(Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{
	unsigned char kernel_sectors;
	Elf32_Half p_filesz;

	p_filesz = (kernel_phdr)->p_filesz;

	/*If p_filesz is > SECTOR_SIZE, we need to write the buffer into imagefile data_blocks times*/
	if (p_filesz % SECTOR_SIZE != 0)
	{
		/*Ceiling operation*/
		kernel_sectors = (p_filesz / SECTOR_SIZE) + 1;
	}

	return kernel_sectors;
}

/* Writes the kernel to the image file */
void write_kernel(FILE **imagefile, FILE *kernelfile, Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{
	Elf32_Half p_offset, p_filesz;
	unsigned char kernel_sectors;

	kernel_sectors = count_kernel_sectors(kernel_header, kernel_phdr);
	p_offset = (kernel_phdr)->p_offset;
	p_filesz = (kernel_phdr)->p_filesz;

	unsigned char buffer[kernel_sectors * SECTOR_SIZE];

	fseek(kernelfile, p_offset, SEEK_SET);

	for (int i = 0; i < kernel_sectors * SECTOR_SIZE; i++)
	{
		buffer[i] = 0;
	}

	/**/
	fread(buffer, p_filesz, 1, kernelfile);
	fwrite(buffer, sizeof(buffer), 1, *imagefile);
}

/* Records the number of sectors in the kernel */
void record_kernel_sectors(FILE **imagefile, Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr, int num_sec)
{
	unsigned char num = count_kernel_sectors(kernel_header, kernel_phdr);
	fseek(*imagefile, 2, SEEK_SET);
	fwrite((const void *)&num, sizeof(char), 1, *imagefile);
}

/* Prints segment information for --extended option */
void extended_opt(Elf32_Phdr *bph, int k_phnum, Elf32_Phdr *kph, int num_sec)
{

	printf("0x%04x: bootblock\n", (bph)->p_vaddr);
	printf("\t segment 0\n");
	printf("\t\t offset 0x%04x\t\t vaddr 0x%04x\n", (bph)->p_offset, (bph)->p_vaddr);
	printf("\t\t filesz 0x%04x\t\t memsz 0x%04x\n", (bph)->p_filesz, (bph)->p_memsz);
	printf("\t\t writing 0x%04x bytes\n", (bph)->p_filesz);
	printf("\t\t padding up to 0x%04x\n", SECTOR_SIZE);

	printf("0x%04x: kernel\n", (kph)->p_vaddr);
	printf("\t segment 0\n");
	printf("\t\t offset 0x%04x\t\t vaddr 0x%04x\n", (kph)->p_offset, (kph)->p_vaddr);
	printf("\t\t filesz 0x%04x\t\t memsz 0x%04x\n", (kph)->p_filesz, (kph)->p_memsz);
	printf("\t\t writing 0x%04x bytes\n", (kph)->p_filesz);
	printf("\t\t padding up to 0x%04x\n", (num_sec + 1) * SECTOR_SIZE);

	printf("os_size: %d sectors\n", num_sec);
}
// more helper functions...

/* MAIN */
int main(int argc, char **argv)
{
	FILE *kernelfile, *bootfile, *imagefile;				// file pointers for bootblock,kernel and image
	Elf32_Ehdr *boot_header = malloc(sizeof(Elf32_Ehdr));	// bootblock ELF header
	Elf32_Ehdr *kernel_header = malloc(sizeof(Elf32_Ehdr)); // kernel ELF header

	Elf32_Phdr *boot_program_header;   // bootblock ELF program header
	Elf32_Phdr *kernel_program_header; // kernel ELF program header
	int num_sec = 0;

	/* build image file */
	imagefile = fopen("image", "wb");

	if (imagefile == NULL)
	{
		printf("[ERROR] Something wrong creating image file\n");
		return 1;
	}

	/* read executable bootblock file */
	boot_program_header = read_exec_file(&bootfile, argv[1], &boot_header);

	/* write bootblock */
	write_bootblock(&imagefile, bootfile, boot_header, boot_program_header);

	/* read executable kernel file */
	kernel_program_header = read_exec_file(&kernelfile, argv[2], &kernel_header);

	/* write kernel segments to image */
	write_kernel(&imagefile, kernelfile, kernel_header, kernel_program_header);

	/* tell the bootloader how many sectors to read to load the kernel */
	record_kernel_sectors(&imagefile, kernel_header, kernel_program_header, num_sec);

	/* check for  --extended option */
	if (argc == 4)
	{
		if (!strncmp(argv[3], "--extended", 11))
		{
			/* print info */
			num_sec = count_kernel_sectors(kernel_header, kernel_program_header);
			extended_opt(boot_program_header, (kernel_header)->e_phnum, kernel_program_header, num_sec);
		}
	}

	printf("[SUCCESS] Image file correctly build\n");

	/*close files*/
	fclose(bootfile);
	fclose(kernelfile);
	fclose(imagefile);

	return 0;
} // ends main()
