#pragma once

#include <string>
#include <vector>



namespace p95
{
	namespace bf
	{
		struct SimConfig
		{
			int intructionsPerSec;
			size_t ticks;
			int maxDataMemorySize;
			int maxProgramMemorySize;

			// Some constants
			static const int MAX_INSTR_PER_SEC = 50;
		};

		enum class MachineState
		{
			READY,
			RUNNING,
			HALTED,
		};

		class BF_Machine
		{
		public:

			BF_Machine() = default;

			void init(SimConfig* config);
			void parseSource(const std::string& source);
			void writeToStdInBuffer(const std::string& val);
			void setState(MachineState newState);
			
			void tick();
			void reset();
			void clearDataMemory();
			void clearIOBuffers();
			void executeInstruction();

			const MachineState getState() const;
			const size_t getTicks() const;
			const size_t getProgMemoSize() const;
			const size_t getDataMemoSize() const;
			const size_t getDataMemoCapacity() const;
			const unsigned int getDataPtr() const;
			const unsigned int getInstructionPtr() const;
			const std::string& getStdIn() const;
			const std::string& getStdOut() const;
			const size_t getStdInSize() const;
			const size_t getStdOutSize() const;
			const char* getDataMemory() const;
			const char* getProgMemory() const;
			const char getCurrentInstruction() const;
			


		public:

			static const size_t MAX_STD_IN_SIZE = 32;
			static const size_t MAX_STD_OUT_SIZE = 32;
			static const size_t MAX_PROG_SOURCE_LEN = 10240;
			
			std::string m_sourceBuffer;

			
		private:

			SimConfig* m_config;

			MachineState m_state;
			size_t m_ticks;
			unsigned char m_currentInstruction;
			std::string m_progMem;
			std::vector<char> m_dataMemory;
			unsigned int m_dataMemoryPtr;
			unsigned int m_instructionPtr;
			std::string m_stdIn;
			std::string m_stdOut;

		};

		/******************************************************************************/
		const char* stateToStr(MachineState state);
	}
}