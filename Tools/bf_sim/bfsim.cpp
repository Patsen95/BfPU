#include "bfsim.h"

#include <iostream>


namespace p95
{
	namespace bf
	{
		void BF_Machine::init(SimConfig* config)
		{
			m_config = config;
			
			m_state = MachineState::READY;
			m_sourceBuffer.reserve(MAX_PROG_SOURCE_LEN);
			m_dataMemoryPtr = 0;
			m_instructionPtr = 0;
			m_dataMemory = std::vector<char>(m_config->maxDataMemorySize, (char)0);
			m_progMem = std::string();
			m_stdIn = std::string();
			m_stdOut = std::string();
			m_stdIn.reserve(MAX_STD_IN_SIZE + 1);
			m_stdOut.reserve(MAX_STD_OUT_SIZE);

			m_currentInstruction = (char)0;
		}

		void BF_Machine::parseSource(const std::string& source)
		{ 
			const std::string _SYNTAX = "><+-.,[]";

			m_progMem = source;
			m_progMem.erase(std::remove_if(m_progMem.begin(), m_progMem.end(), [&_SYNTAX](const char& c) {
				return _SYNTAX.find(c) == std::string::npos;
			}), m_progMem.end());

			m_currentInstruction = m_progMem[m_instructionPtr];
		}

		void BF_Machine::writeToStdInBuffer(const std::string& val)
		{
			if(val.length() > MAX_STD_IN_SIZE)
				return;
			m_stdIn = val;
		}

		void BF_Machine::setState(MachineState newState)
		{
			m_state = newState;
		}

		/******************************************************************************/
		void BF_Machine::tick()
		{
			if(m_instructionPtr >= getProgMemoSize())
			{
				m_state = MachineState::HALTED;
				return;
			}
			executeInstruction();
			m_ticks++;
			m_currentInstruction = m_progMem[m_instructionPtr];
		}

		void BF_Machine::reset()
		{
			m_state = MachineState::READY;
			m_ticks = 0;

			m_instructionPtr = 0;
			m_currentInstruction = (char)0;
			m_progMem = std::string();

			clearDataMemory();
			clearIOBuffers();
		}

		void BF_Machine::clearDataMemory()
		{
			m_dataMemory = std::vector<char>(m_config->maxDataMemorySize, (char)0);
			m_dataMemoryPtr = 0;
		}

		void BF_Machine::clearIOBuffers()
		{
			m_stdIn = std::string();
			m_stdOut = std::string();
		}

		void BF_Machine::executeInstruction()
		{
			static int _jumps = 0;
			static std::vector<unsigned int> _jmpStack;

			if(_jumps > 0)
			{
				if(m_currentInstruction == '[') _jumps++;
				else if(m_currentInstruction == ']') _jumps--;

				m_instructionPtr++;
				return;
			}

			switch(m_currentInstruction)
			{
				case '>':
					if(m_dataMemoryPtr < getDataMemoCapacity()) m_dataMemoryPtr++;
					break;

				case '<':
					if(m_dataMemoryPtr > 0) m_dataMemoryPtr--;
					break;

				case '+':
					m_dataMemory[m_dataMemoryPtr]++;
					break;

				case '-':
					m_dataMemory[m_dataMemoryPtr]--;
					break;

				case '.':
					if(m_stdOut.length() < MAX_STD_OUT_SIZE)
						m_stdOut.push_back(m_dataMemory[m_dataMemoryPtr]);
					break;

				case ',':
					if(m_stdIn.length() > 0)
					{
						m_dataMemory[m_dataMemoryPtr] = m_stdIn[0];
						m_stdIn.pop_back();
					}
					break;

				case '[':
					if(m_dataMemory[m_dataMemoryPtr] == 0) _jumps = 1;
					else 
						_jmpStack.push_back(m_instructionPtr);

					break;

				case ']':
					if(m_dataMemory[m_dataMemoryPtr] != 0)
					{
						if(!_jmpStack.empty()) // TODO: Proper error message
							m_instructionPtr = _jmpStack.back();
						
						return;
					}
					else
					{
						if(!_jmpStack.empty()) // TODO: Proper error message
							_jmpStack.pop_back();
					}
					break;
			}
			if(m_instructionPtr < getProgMemoSize())
				m_instructionPtr++;
		}

		/******************************************************************************/
		const MachineState BF_Machine::getState() const
		{
			return m_state;
		}

		const size_t BF_Machine::getTicks() const
		{
			return m_ticks;
		}

		const size_t BF_Machine::getProgMemoSize() const
		{
			return m_progMem.length();
		}

		const size_t BF_Machine::getDataMemoSize() const
		{
			return m_dataMemory.size();
		}

		const size_t BF_Machine::getDataMemoCapacity() const
		{
			return m_dataMemory.capacity();
		}

		const unsigned int BF_Machine::getDataPtr() const
		{
			return m_dataMemoryPtr;
		}

		const unsigned int BF_Machine::getInstructionPtr() const
		{
			return m_instructionPtr;
		}

		const std::string& BF_Machine::getStdIn() const
		{
			return m_stdIn;
		}

		const std::string& BF_Machine::getStdOut() const
		{
			return m_stdOut;
		}

		const size_t BF_Machine::getStdInSize() const
		{
			return m_stdIn.length();
		}

		const size_t BF_Machine::getStdOutSize() const
		{
			return m_stdOut.length();
		}

		const char* BF_Machine::getDataMemory() const
		{
			return m_dataMemory.data();
		}

		const char* BF_Machine::getProgMemory() const
		{
			return m_progMem.c_str();
		}

		const char BF_Machine::getCurrentInstruction() const
		{
			return m_currentInstruction;
		}

		/******************************************************************************/
		const char* stateToStr(MachineState state)
		{
			switch(state)
			{
				case MachineState::READY: return "READY";
				case MachineState::RUNNING: return "RUNNING";
				case MachineState::HALTED: return "HALT";
				default: return "UNKNOWN";
			}
		}
	}
}