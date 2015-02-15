/*-----------------------------------------------------------------------------
@file

Definition of all DSP routines which perform modualtion
The naming conventions are as follows:
Class names: ClassName
Class member data : dataMember
Class member function: functionMember



------------------------------------------------------------------------------*/

#include <complex>
#include <cstdint>

namespace dsptl_private
{
	/*-----------------------------------------------------------------------------
	Modulator nominal vlaues for different type of output

	------------------------------------------------------------------------------*/
	template<class T>
	struct ModAmplitude;

	template<>
	struct ModAmplitude < int16_t >
	{
		static const int16_t value = 8192;
	};

	template<>
	struct ModAmplitude < int8_t >
	{
		static const int8_t value = 32;
	};

	template<>
	struct ModAmplitude < int32_t >
	{
		static const int32_t value = 8192;
	};

	template<>
	struct ModAmplitude < float >
	{
		ModAmplitude(){ value = 0.707f; }
		float value;
	};
	template<>
	struct ModAmplitude < double >
	{
		ModAmplitude(){ value = 0.707; }
		double value ;
	};
}

namespace dsptl
{

	/*-----------------------------------------------------------------------------
	SDPSK modulator

	Takes a bit pattern and generates the modulation symbols for an SDPSK modulation\n
	The output of the modulator is always complex with the type specified as template
	parameter

	@tparam T Type of each of the output of the complex sample


	------------------------------------------------------------------------------*/
	template<class T>
	class ModulatorSdpsk
	{
	public:
		/// Default constructor
		ModulatorSdpsk()
		{
			//static_assert(std::is_signed<T>::value);
			T amplitude;
			amplitude = dsptl_private::ModAmplitude < T > {}.value;

			map[0] = { amplitude, amplitude };
			map[1] = { static_cast<T>(-amplitude), amplitude };
			map[2] = {  static_cast<T>(-amplitude),  static_cast<T>(-amplitude) };
			map[3] = { amplitude,  static_cast<T>(-amplitude) };

		}

		/// Performs the modulation as required
		void step(const std::vector<int8_t> & bits , std::vector<std::complex<T> > & out)
		{
			const int N = 4;
			assert(bits.size() == out.size());
			for (size_t i = 0; i < bits.size(); i++)
			{
				// Generates a state from 0 to 3
				state += (bits[i] > 0)? 1 : (N-1);
				state = state % N;
				// Map the state to the correct amplitude
				std::complex<T> tmp = map[state];
				out[i] = tmp;
				
			}
		}

		/// Resets the state to a known value
		void reset()
		{
			state = 0;
		}


	private:
		int state = 0;
		std::complex<T> map[4];


	};


}


