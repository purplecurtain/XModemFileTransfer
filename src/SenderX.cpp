//============================================================================
//
//% Student Name 1: Nathan Tannar
//% Student 1 #: 301258264
//% Student 1 userid (email): ntannar@sfu.ca
//
//% Student Name 2: Shahira Afrin
//% Student 2 #: 123456782
//% Student 2 userid (email): stu2 (stu2@sfu.ca)
//
//% Below, edit to list any people who helped you with the code in this file,
//%      or put 'None' if nobody helped (the two of) you.
//
// Helpers: _everybody helped us/me with the assignment (list names or put 'None')__
//
// Also, list any resources beyond the course textbooks and the course pages on Piazza
// that you used in making your submission.
//
// Resources:  ___________
//
//%% Instructions:
//% * Put your name(s), student number(s), userid(s) in the above section.
//% * Also enter the above information in other files to submit.
//% * Edit the "Helpers" line and, if necessary, the "Resources" line.
//% * Your group name should be "P1_<userid1>_<userid2>" (eg. P1_stu1_stu2)
//% * Form groups as described at:  https://courses.cs.sfu.ca/docs/students
//% * Submit files to courses.cs.sfu.ca
//
// File Name   : SenderX.cc
// Version     : September 3rd, 2017
// Description : Starting point for ENSC 351 Project
// Original portions Copyright (c) 2017 Craig Scratchley  (wcs AT sfu DOT ca)
//============================================================================

#include <iostream>
#include <stdint.h> // for uint8_t
#include <string.h> // for memset()
#include <errno.h>
#include <fcntl.h>	// for O_RDWR

#include "myIO.h"
#include "SenderX.h"

using namespace std;

SenderX::SenderX(const char *fname, int d)
:PeerX(d, fname), bytesRd(-1), blkNum(255)
{
}

//-----------------------------------------------------------------------------

/* tries to generate a block.  Updates the
variable bytesRd with the number of bytes that were read
from the input file in order to create the block. Sets
bytesRd to 0 and does not actually generate a block if the end
of the input file had been reached when the previously generated block was
prepared or if the input file is empty (i.e. has 0 length).
*/
void SenderX::genBlk(blkT blkBuf)
{

//	Each block of the transfer looks like:
//	          <SOH><blk #><255-blk #><--128 data bytes--><cksum>
//	blkBuf is the reverse of that

	if (-1 == (bytesRd = myRead(transferringFileD, &blkBuf[1], CHUNK_SZ )))
		ErrorPrinter("myRead(transferringFileD, &blkBuf[1], CHUNK_SZ )", __FILE__, __LINE__, errno);

	else if (0 == bytesRd){ //end of file or empty file
		blkBuf[131] = EOT;
	}
	else {

		for (ssize_t i = bytesRd; i < 128; i++)
			blkBuf[i] = CTRL_Z;

		/*The checksum is based on the value of all the bytes in the chunk added together.  For
		 example, if the [last] five bytes in the chunk were 45, 12, 64, 236, 173 and the
		 other 123 bytes were zeroes, the sum would be 0+0+...+0+45+12+64+236+173 = 530.
		 However, one must repeatedly subtract 256 from the sum until the result, the
		 checksum, is between 0 and 255.  In this case, the checksum
		 would be 530 - 256 - 256 = 18.*/

		uint8_t checksum = 0;

		for (ssize_t i = 1; i <= bytesRd; i++ )
		{
			checksum += blkBuf[i];
		}
		while (checksum > 255)
			checksum -= 255;
		blkBuf[0] = checksum;

		/*The 1's complement of a byte (to make life easy) is simply 255 minus the
		 byte.  For example, if you had to take the 1's complement of 142, the answer
		 would be 255 - 142 = 133. */
		blkBuf[129] = 255 - blkNum;
		blkBuf[130] = blkNum;
		blkBuf[131] = SOH;

	}

}

void SenderX::sendFile()
{
	transferringFileD = myOpen(fileName, O_RDWR, 0);
	if(transferringFileD == -1) {
		cout /* cerr */ << "Error opening input file named: " << fileName << endl;
		result = "OpenError";
	}
	else {
		cout << "Sender will send " << fileName << endl;

//		The sender sends:
//		    1. an SOH byte                             {1 byte}
//
//		    2. the block number                        {1 byte}
//
//		    3. the 1's complement of the block number  {1 byte}
//
//		    4. the chunk                            {128 bytes}
//
//		    5. the checksum                            {1 byte}
//
//		    The above five things are called the block.

		blkNum = 1;

		// do the protocol, and simulate a receiver that positively acknowledges every
		//	block that it receives.

		// assume 'C' or NAK received from receiver to enable sending with CRC or checksum, respectively
		genBlk(blkBuf); // prepare 1st block
		while (bytesRd)
		{
			blkNum ++; // 1st block about to be sent or previous block was ACK'd

			// ********* fill in some code here to send a block ***********
			for (ssize_t i = 0; i < 132; i++)
				sendByte(blkBuf[i]);


			// assume sent block will be ACK'd
			genBlk(blkBuf); // prepare next block
			// assume sent block was ACK'd
		};
		// finish up the protocol, assuming the receiver behaves normally
		// ********* fill in some code here **********
		// we need to send a final EOT
		sendByte(EOT);



		//(myClose(transferringFileD));
		if (-1 == myClose(transferringFileD))
			ErrorPrinter("myClose(transferringFileD)", __FILE__, __LINE__, errno);
		result = "Done";
	}
}

