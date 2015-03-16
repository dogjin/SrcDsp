/***********************************************************************************************************************
                    
File Name:			SDLViterbiDecFix.c
Origination Date:	April 6, 2010
Author:				Torbjorn Larsson
Last Modified:		April 6, 2010

% Argon ST Proprietary Data

DESCRIPTION:
	Viterbi decoder for the OG2 SDL convolutional code with tail-biting and puncturing. De-puncturing is included
	in the decoder. When the Viterbi recursion reaches the end of the block after 'infoLen' steps, it wraps around
	and continues the recursion from the beginning of the block for another 2*L steps. After the last decoding step
	has been executed, the state with maximum path metric is found and the survivor memory is backtraced starting
	from this state. During the first L-6 traceback steps, no bit decisions are released. During the next L traceback
	steps, bit decisions corresponding to the first L bits in the block are released. During the final infoLen-L
	traceback steps, bit decisions corresponding to the last infoLen-L bits in the block are released. The total
	number of returned info bits is 'infoLen'.

MATLAB USAGE:
	decBits = SDLViterbiDecFix( infoLen, codeLen, softIn, L )

INPUT ARGUMENTS:
	infoLen			Information block length.
					MANDATORY.

	codeLen			Code block length (punctured code block).
					MANDATORY.

	SoftIn			Soft code bit decisions {vector of length codeLen). Assumed bitwidth: 8.
					MANDATORY.
 
	L				Parameter that determines the behavior during traceback (see description above).
					MANDATORY.

OUTPUT ARGUMENTS:
	decBits			Decoded info bits {row vector}.

***********************************************************************************************************************/
// INCLUDES:
#include <math.h>

// LOCAL DEFINES, ENUMERATIONS, TYPEDEFS:
#define STATE_LEN 6 // number of memory cells in the state
#define NUM_STATES 64
#define NUM_BUTTERFLIES 32

// LOCAL PROTOTYPES:

// LOCAL STRUCTURES:

// GLOBAL, EXTERNAL & STATIC DEFINITIONS:

// Lookup table specifying the branch label for the upper state transition in each butterfly.
// A branch label is the code bits on the branch (state transition) interpreted as an integer.
// If the two code bits are c1 and c2 (where c1 is transmitted first), the branch label is c2*2 + c1.
// Only the label for the upper branch in each butterfly needs to be specified, since this label
// uniqely determines the labels for the three other branches.
static const unsigned labels[NUM_BUTTERFLIES] = {
	0, 1, 3, 2, 3, 2, 0, 1, 0, 1, 3, 2, 3, 2, 0, 1, 2, 3, 1, 0, 1, 0, 2, 3, 2, 3, 1, 0, 1, 0, 2, 3 };

/***********************************************************************************************************************
FUNCTION NAME: dePunct

DESCRIPTION:
	De-puncturing operation. The puncturing pattern is 110110.
 
SYNOPSIS:
	dePunct( codeLen, SIBuff1, SIBuff2 )
    
INPUT ARGUMENTS:
	codeLen			Code block length (before de-puncturing).

	SIBuff1			Pointer to soft input buffer 1 (before de-puncturing).

	SIBuff2			Pointer to soft input buffer 2 (after de-puncturing, output from this function).

RETURNS:			None.

*/
void dePunct( int codeLen, int *SIBuff1, int *SIBuff2 )
{
	int n, p;

	p = 0; // pointer to next position in SI2 (output) buffer
	for ( n = 0; n < codeLen; n+=2 ) { // for each pair of soft inputs before de-puncturing...
		SIBuff2[p] = SIBuff1[n];
		SIBuff2[p+1] = SIBuff1[n+1];
		SIBuff2[p+2] = 0;
		p += 3;
	}
}

/***********************************************************************************************************************
FUNCTION NAME: branchMetric

DESCRIPTION:
	Calculate a branch metric.
 
SYNOPSIS:
	bm = branchMetric( soft1, soft2, label )
    
INPUT ARGUMENTS:
	soft1			First soft input. Bitwidth: 8 (signed).

	soft2			Second soft input. Bitwidth: 8 (signed).

	label			Branch label. The branch label is the code bits on the branch (state transition) interpreted
					as an integer. If the two code bits are c1 and c2 (where c1 is transmitted first), the branch
					label is c2*2 + c1.

RETURNS:
	bm				The computed branch metric. Bitwidth: 6 (signed).

*/
int branchMetric( int soft1, int soft2, unsigned label )
{
	unsigned bit1, bit2; // bits in the branch label
	int bm; // branch metric
	
	// Extract the two bits in the branch label
	bit1 = label & 1;
	bit2 = (label >> 1) & 1;
	
	// Convert the bits to antipodal form (+/-1) and compute the branch metric before truncation
	bm = (2*bit1-1)*soft1 + (2*bit2-1)*soft2; // bitwidth = 9
	
	// Round off two bits
	bm = (bm + 2) >> 2; // bitwidth = 7
	
	// Remove one bit by saturation
	if (bm > 31) bm = 31;
	if (bm < -31) bm = -31;
	
	// Return the branch metric
	return bm; // bitwidth = 6
}

/***********************************************************************************************************************
FUNCTION NAME: ACS

DESCRIPTION:
	Add-compare-select operation.
 
SYNOPSIS:
	pm = ACS( pm00, pm01, bm0, bm1, &surv )
    
INPUT ARGUMENTS:
	pm00			Path metric for first predecessor state. Bitwidth: 9 (unsigned).

	pm01			Path metric for second predecessor state. Bitwidth: 9 (unsigned).

	bm0				Branch metric for the branch from the first predecessor state. Bitwidth: 6 (signed).

	bm1				Branch metric for the branch from the second predecessor state. Bitwidth: 6 (signed).

	surv			Survivor info (one bit).

OUTPUT ARGUMENTS:
	dataBuff		Decoded data (info) bits {vector of length infoLen}.
 	    
RETURNS:
	pm				The surviving path metric.

*/
int ACS( unsigned pm00, unsigned pm01, int bm0, int bm1, unsigned *surv )
{
	int cm0, cm1; // candidate metrics
	int pm; // selected metric
	
	// Add to get candidate metrics
	// Note: need to append a sign bit to the path metric before adding since result can be negative
	cm0 = pm00 + bm0; // candidate metric from s00 to new state (signed number with bitwidth = 10)
	cm1 = pm01 + bm1; // candidate metric from s01 to new state

	// Compare candidate metrics to select survivor path and metric
	// Note: remove sign bit from survivor metric (it is now guaranteed to be non-negative)
	if (cm0 > cm1) {
		*surv = 0;
		pm = (unsigned) cm0; // bitwidth = 9
	}
	else {
		*surv = 1;
		pm = (unsigned) cm1; // bitwidth = 9
	}
	
	// Return the surviving metric
	return pm;
}

/***********************************************************************************************************************
FUNCTION NAME: viterbi

DESCRIPTION:
	Viterbi decoder for a rate-1/2, 64-state, tail-biting convolutional code with puncturing.
 
SYNOPSIS:
	viterbi( infoLen, codeLen, SIBuff1, L, dataBuff )
    
INPUT ARGUMENTS:
	infoLen			Information block length.

	codeLen			Code block length (punctured). This is also the number of the soft inputs.

	SIBuff1			Pointer to buffer with soft code bit decisions {buffer size = 2*infoLen).

	L				Traceback parameter.

OUTPUT ARGUMENTS:
	dataBuff		Pointer to decoded data (info) buffer {buffer size = infoLen}.
 	    
RETURNS:
	None.

*/
void viterbi( unsigned infoLen, unsigned codeLen, int *SIBuff1, unsigned L, unsigned *dataBuff )
{	
	unsigned numStep; // number of Viterbi recursion steps
	unsigned traceLen; // traceback length (number of traceback steps)
	int *SIBuff2; // pointer soft input buffer 2 (after de-puncturing)
	int *BMBuff; // pointer to branch metric buffer
	unsigned *PMBuff; // pointer to path metric buffers (old/newpm)
	unsigned char *SMBuff; // pointer to survivor memory buffer
	unsigned k, n; // counters
	int bm; // branch metric
	unsigned s, sMax; // states
	unsigned s00, s01, s10, s11; // states in the Viterbi butterfly
	unsigned old, newpm, tmp; // pointers to "old" and "new" path metric buffers
	unsigned writePnt, readPnt; // pointers to suvivor memory
	unsigned dataPnt; // pointer to decoded data memory
	unsigned bmPnt; // pointer to first branch metric
	unsigned c; // branch label, i.e. code bits on one branch interpreted as an integer
	unsigned pm00, pm01, pm10, pm11, pm, pmMax; // path metrics
	unsigned pmOffs; // path metric offset
	unsigned normAlert; // metric normalization alert status (one bit)
	unsigned surv; // one bit indicating the surviving path (path from s00 or path from s01)
	unsigned bit; // data bit released during traceback
	
	// Initialize parameters that depend on L
	numStep = infoLen + 2*L; // number of Viterbi recursion steps
	traceLen = infoLen + L - STATE_LEN; // traceback length
		
	// Allocate buffers
	SIBuff2 = new int[ 2 * infoLen ]; // soft input 2 (bitwidth: 8)
	BMBuff = new int[ 4 * infoLen  ]; // branch metrics (bitwidth: 6)
	PMBuff = new unsigned int[2 * NUM_STATES  ]; // path metrics (bitwidth: 9)
	SMBuff = new unsigned char[ traceLen * NUM_STATES  ]; // survivor memory (bitwidth: 1)
	
	// De-puncturing of soft inputs (de-punctured soft decisions are stored in SIBuff2)
	dePunct( codeLen, SIBuff1, SIBuff2 );
	
	// Compute branch metrics (6-bit signed numbers)
	for ( n = 0; n < 2*infoLen-1; n+=2 ) { // for each pair of soft decisions...
		bm = branchMetric( SIBuff2[n], SIBuff2[n+1], 3 ); // branch metric for label '11'		
		BMBuff[2*n] = -bm; // label '00'
		BMBuff[2*n+3] = bm; // label '11'
		bm = branchMetric( SIBuff2[n], SIBuff2[n+1], 1 ); // branch metric for label '01'
		BMBuff[2*n+1] = bm; // label '01'
		BMBuff[2*n+2] = -bm; // label '10'
	}
	
	// Path metric initialization
	old = 0; // pointer to "old" path metric buffer
	newpm = NUM_STATES; // pointer to "new" path metric buffer
	for (s = 0; s < NUM_STATES; s++)
		PMBuff[s] = 0;

	// Viterbi recursions
	n = 0; // time index
	writePnt = 0; // write pointer for survivor memory
	normAlert = 0; // metric normalization alert status
	for ( k = 0; k < numStep; k++ ) { // for each decoding step...
		
		// Carry out the Viterbi recursion in the kth decoding step
		bmPnt = n*4; // pointer to first branch metric at time n
		//pmOffs = normAlert? 256 : 0; // set path metric offset for normalization
		pmOffs = 0; // DISABLE NORMALIZATION
		normAlert = 1; // prepare for computing new normalization alert status bit
				
		for ( s00 = 0; s00 < NUM_BUTTERFLIES; s00++ ) { // for each butterfly...
			// Note: variable s00 specifies the upper left state in the butterfly
						
			// Read the old path metrics for state s00 and state s01 (lower left state) and subtract offset
			// Note: result after subtracting offset is guaranteed to be non-negative
			s01 = NUM_BUTTERFLIES | s00; // set msb (number of butterflies is a power of 2)
			pm00 = (unsigned) PMBuff[old+s00] - pmOffs; // path metric for state s00 (9-bit unsigned number)
			pm01 = (unsigned) PMBuff[old+s01] - pmOffs; // path metric for state s01 (9-bit unsigned number)

			// Read the branch metric for this butterfly (for the branch from s00 to s10)
			c = labels[s00]; // get branch label from lookup table
			bm = BMBuff[bmPnt+c]; // branch metric (6-bit signed number)

			// Add-compare-select for the two paths entering state s10 (upper right state)
			pm10 = ACS( pm00, pm01, bm, -bm, &surv ); // 9-bit unsigned number
			s10 = s00 << 1;
			PMBuff[newpm+s10] = pm10; // store survivor metric
			SMBuff[writePnt+s10] = surv; // store survivor info (one bit)

			// Add-compare-select for the two paths entering state s11 (lower right state)
			pm11 = ACS( pm00, pm01, -bm, bm, &surv ); // 9-bit unsigned number
			s11 = s10 | 1; // set lsb
			PMBuff[newpm+s11] = pm11; // store survivor metric
			SMBuff[writePnt+s11] = surv; // store survivor info (one bit)

			// Note: The survivor path information stored at step k corresponds to the data bit at time k-STATE_LEN

			// Update normalization alert status based on the two new metrics
			normAlert = normAlert & (pm10 > 255) & (pm11 > 255);
			
			// Stop if overflow (simulation only)
			//if (pm10 > 511 | pm11 > 511)
			//	mexErrMsgTxt("Overflow in Viterbi decoder");

		}
		
		// Swap "old" and "new" path metric buffers
		tmp = old;
		old = newpm;
		newpm = tmp;
		
		// Step up time index
		n++;
		if (n == infoLen)
			n = 0;
		
		// Step up survivor memory pointer
		// Note: bits stored in the SM buffer during the first L + STATE_LEN steps will be over-written
		if (k >= L+STATE_LEN) // if the number of executed steps exceeds L + STATE_LEN...
			writePnt += NUM_STATES;

	}
	// End of Viterbi recursions
	
	// Find state with maximum path metric
	pmMax = 0; // maximum path metric found so far
	sMax = 0; // state with maximum metric
	for ( s = 0; s < NUM_STATES; s++ ) {
		pm = PMBuff[old+s];
		if (pm > pmMax) {
			pmMax = pm;
			sMax = s;
		}
	}

	// Traceback
	// Note: the last 'STATE_LEN' bits are not returned
	s = sMax; // start from state with maximum metric
	readPnt = (traceLen-1)*NUM_STATES; // pointer to survivor memory
	dataPnt = L - 1; // pointer to next write position in data buffer
	for ( k = 0; k < traceLen; k++ ) {
		bit = (unsigned) SMBuff[readPnt+s]; // read survivor info (this is the oldest bit in the previous state)
		s = (bit << (STATE_LEN-1)) | (s >> 1); // re-construct previous state ADDED parenthesis for clarity
		if (k >= L-STATE_LEN) { // if we have previously back-traced at least L-STATE_LEN steps...
			dataBuff[dataPnt] = bit; // store info bit
			if (dataPnt == 0)
				dataPnt = infoLen - 1;
			else
				dataPnt--;
		}
		readPnt -= NUM_STATES;
	}
	
	// De-allocate memory
	delete[] SIBuff2 ;
	delete[]  BMBuff ;
	delete[]  PMBuff ;
	delete[]  SMBuff ;
}


