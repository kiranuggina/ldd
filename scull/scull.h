#ifndef _SCULL_H_
#define _SCULL_H_

struct scull_dev {
	char   message[256];	/* buffer to hold data */
	short  size_of_message; /* Used to remember the size of the string stored */
	struct cdev cdev;	  	/* Char device structure		*/
};	

#endif /*_SCULL_H_*/
