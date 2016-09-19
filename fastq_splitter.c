#include <zlib.h>
#include <stdio.h>
#include "kseq.h"
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

// Using code from Heng Li's kseq example - http://lh3lh3.users.sourceforge.net/parsefastq.shtml

// declare the type of file handler and the read() function
KSEQ_INIT(gzFile, gzread)

int is_valid_fd(int fd) { // Function to check if file handle is open
	return fcntl(fd,F_GETFD) != 1 || errno != EBADF;
}

int main(int argc, char *argv[])
{
	gzFile fp;
	gzFile out;
	kseq_t *seq;
	int l,a,i=0,c=0,numreads;
	char *prefix, *fname;
	
	while ((a = getopt(argc,argv, "p:n:")) >= 0) {
		switch(a) {
			case 'p': prefix = optarg; break;
			case 'n': numreads = atoi(optarg); break;
		}
	} 

	if (optind == argc) {
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: fastq_splitter [options] <in.fq.gz>\n");
		fprintf(stderr, "Options -p STR Output prefix \n");
		fprintf(stderr, "        -n INT Number of reads per file \n");
		fprintf(stderr,"\n");
		return 1;
	}

	fp = strcmp(argv[optind],"-")? gzopen(argv[optind],"r") : gzdopen(fileno(stdin),"r"); // Handle both stdin and file 
	if (fp == 0) {
		perror("Can't open file ");
	}
	seq = kseq_init(fp); //  initialize seq
	while ((l = kseq_read(seq)) >= 0) { // read sequence
		i += 1;
		if (i % numreads == 1) {
			c += 1;
			if (is_valid_fd((int) out)) gzclose(out);
			fname = malloc(strlen(prefix)+12);
			sprintf(fname,"%s-S%06d.fq.gz",prefix,c); //Set output filename based on the prefix 
			printf("Writing to %s\n",fname);
			out = gzopen(fname,"w");
			free(fname);
		}
		gzprintf(out,"@%s %s\n",seq->name.s,seq->comment.s);
		gzprintf(out,"%s\n",seq->seq.s);
		gzprintf(out,"+\n");
		gzprintf(out,"%s\n",seq->qual.s);
	}
	kseq_destroy(seq); 
	gzclose(fp);  // close input file
	if (is_valid_fd((int) out)) gzclose(out);
	return 0;
}

