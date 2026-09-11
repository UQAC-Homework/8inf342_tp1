#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

namespace
{
	/// Data structure responsible to hold the history of ran commands
	class CommandHistory
	{
	private:
		size_t _next;
		const size_t _size;
		char** _end;
		char** _commands;

	public:
		explicit CommandHistory(const size_t size) : _size(size)
		{
			_commands = new char*[_size]();
			_next = 0;
			_end = &_commands[size];
		}

		/// Adds the given command to the history
		void add(const char* command)
		{
			// If buffer filled, shift all
			if (_next == _size)
			{
				for (int i = 0; i < _next - 1; i++)
					_commands[i] = _commands[i + 1];
				
				_commands[_next - 1] = nullptr;
				_next--;
				//add(command);
				//return;
			}

			const auto entry = new char[strlen(command) + 1];
			strcpy(entry, command);

			_commands[_next] = entry;
			_next++;
		}

		/// Prints all the commands to the given output
		void print(std::ostream& output) const
		{
			for (auto i = 0; i < _next; i++)
			{
				output << _commands[i] << std::endl;
			}
		}

		~CommandHistory()
		{
			for (auto i = 0; i < _next; i++)
				delete[] _commands[i];

			delete[] _commands;
		}
	};
}

/// Function responsible to execute the given command
static void execute_command(char* command)
{
	// Split arguments
	std::vector<char*> args;
	char* command_name = strtok(command, " ");
	char* current_keyword = command_name;

	while (current_keyword != nullptr)
	{
		args.push_back(current_keyword);
		current_keyword = strtok(nullptr, " ");
	}

	// Prepare arguments for execution
	const auto argv = new char*[args.size() + 1];

	for (int k = 0; k < args.size(); k++)
		argv[k] = args[k];

	argv[args.size()] = nullptr;

	// Run command
	execvp(command_name, argv);

	delete[] argv;
}

int main()
{
	auto history = CommandHistory(5);

	while (true)
	{
		std::string result;

		std::cout << "GauthierChalons< ";
		char command[128];
		std::cin.getline(command, 128);

		// If empty, skip
		if (strlen(command) == 0)
			continue;

		// If stop command, exit
		if (strcmp(command, "stop") == 0)
			return 0;

		// If history command, display command history
		if (strcmp(command, "historique") == 0)
		{
			history.print(std::cout);
			continue;
		}

		// Copy original command
		const auto copy_command = static_cast<char*>(malloc(128));
		strcpy(copy_command, command);

		// 
		if (command[0] == '.')
		{
			const auto shell_command = strtok(command, " ");
			std::system(shell_command);
			continue;
		}

		// Fork
		const pid_t child_pid = fork();

		// If failed, exit
		if (child_pid < 0)
		{
			perror("Could not fork");
			return -1;
		}

		// If as child
		if (child_pid == 0)
		{
			execute_command(command);
			return 0;
		}

		// If child failed, exit
		if (waitpid(child_pid, nullptr, 0) < 0)
			return -1;

		history.add(copy_command);
	}

	return 0;
}
