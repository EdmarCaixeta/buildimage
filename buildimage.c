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

	/*When the ELF file does not represent an executable program, the ELF header does the job of a Program header*/
	else
	{
	}
}

/* Writes the bootblock to the image file */
void write_bootblock(FILE **imagefile, FILE *bootfile, Elf32_Ehdr *boot_header, Elf32_Phdr *boot_phdr)
{
}

/* Writes the kernel to the image file */
void write_kernel(FILE **imagefile, FILE *kernelfile, Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{
}

/* Counts the number of sectors in the kernel */
int count_kernel_sectors(Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr)
{
	return 0;
}

/* Records the number of sectors in the kernel */
void record_kernel_sectors(FILE **imagefile, Elf32_Ehdr *kernel_header, Elf32_Phdr *kernel_phdr, int num_sec)
{
}

/* Prints segment information for --extended option */
void extended_opt(Elf32_Phdr *bph, int k_phnum, Elf32_Phdr *kph, int num_sec)
{

	/* print number of disk sectors used by the image */

	/* bootblock segment info */

	/* print kernel segment info */

	/* print kernel size in sectors */
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

	/* build image file */
	imagefile = fopen("image", "wb");

	if (imagefile == NULL)
	{
		printf("[ERROR] Something wrong creating image file\n");
		return 1;
	}

	/* read executable bootblock file */
	boot_program_header = read_exec_file(&bootfile, argv[2], &boot_header);

	/* write bootblock */

	/* read executable kernel file */

	/* write kernel segments to image */

	/* tell the bootloader how many sectors to read to load the kernel */

	/* check for  --extended option */
	if (!strncmp(argv[1], "--extended", 11))
	{
		/* print info */
	}

	return 0;
} // ends main()
