#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <chrono>
#include <random>

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

		size_t _command_count;

	public:
		explicit CommandHistory(const size_t size) : _size(size)
		{
			_commands = new char*[_size]();
			_next = 0;
			_end = &_commands[size];
			_command_count = 0;
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
			}

			const auto entry = new char[strlen(command) + 1];
			strcpy(entry, command);

			_commands[_next] = entry;
			_next++;
			_command_count++;
		}

		/// Prints all the commands to the given output
		void print(std::ostream& output) const
		{
			const auto count = std::min(_command_count, _size);

			for (auto i = 0; i < _next; i++)
			{
				output << (count - i);
				output << "\t";
				output << _commands[i];
				output << std::endl;
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
	const auto code = execvp(command_name, argv);

	if (code != 0)
	{
		const std::string str_command(command);
		perror(("Failed to execute the command '" + str_command + "'").c_str());
	}

	delete[] argv;
}

/// Function responsible to run the automated benchmarking instances (Part 2)
static void run_automated_instance(int instance_id, int total_commands, int history_interval)
{
	auto history = CommandHistory(5);
    
	// Pool of commands to execute randomly. Avoided interactive commands like "man" to prevent blocking.
	std::vector<std::string> command_pool = {
		"ls -l", "pwd", "ls -a", "mkdir testdir", "rmdir testdir"
	};

	std::string all_cmds_filename = "toutes_les_commandes_instance" + std::to_string(instance_id) + ".txt";
	std::ofstream all_cmds_file(all_cmds_filename);

	std::cout << "Starting Instance " << instance_id << " (" << total_commands << " commands)...\n";
    
	// Start timer for the report
	auto start_time = std::chrono::high_resolution_clock::now();

	for (int i = 1; i <= total_commands; i++)
	{
		// Pick a random command
		std::string cmd_str = command_pool[rand() % command_pool.size()];
		all_cmds_file << cmd_str << "\n";

		char command[128];
		strcpy(command, cmd_str.c_str());

		// Fork
		const pid_t child_pid = fork();

		// If failed, exit
		if (child_pid < 0)
		{
			perror("Could not fork");
			return;
		}

		// If as child
		if (child_pid == 0)
		{
			execute_command(command);
			exit(0); // Ensure child exits
		}

		// Wait for child
		waitpid(child_pid, nullptr, 0);

		// Record in history
		std::string history_entry = std::string(command) + "\t" + std::to_string(child_pid);
		history.add(history_entry.c_str());

		// Trigger history export at specific intervals
		if (i % history_interval == 0)
		{
			std::string hist_filename = "historique" + std::to_string(instance_id) + "_" + std::to_string(i) + ".txt";
			std::ofstream output(hist_filename);
			history.print(output);
			output.close();
			std::cout << "[INFO] Saved history to " << hist_filename << " (Command " << i << ")\n";
		}
	}

	all_cmds_file.close();
    
	// End timer and print the execution duration
	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;
	std::cout << "Instance " << instance_id << " finished in " << diff.count() << " seconds.\n\n";
}

int main()
{
	// Initialize random seed for automated instances
	srand(time(nullptr));

	std::cout << "Select mode:\n";
	std::cout << "1. Interactive mode (Part 1)\n";
	std::cout << "2. Run Instance 1 (100 commands)\n";
	std::cout << "3. Run Instance 2 (500 commands)\n";
	std::cout << "Choice: ";
	
	int choice;
	std::cin >> choice;
	std::cin.ignore(); // Flush newline character

	if (choice == 2)
	{
		run_automated_instance(1, 100, 50);
		return 0;
	}
	else if (choice == 3)
	{
		run_automated_instance(2, 500, 100);
		return 0;
	}

	// ==========================================
	// ORIGINAL INTERACTIVE CODE BELOW (PART 1)
	// ==========================================

	auto history = CommandHistory(5);

	while (true)
	{
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

			std::ofstream output;
			output.open("historique.txt");
			history.print(output);
			output.close();
			continue;
		}

		// Run program
		if (command[0] == '.')
		{
			const auto shell_command = strtok(command, " ");
			std::system(shell_command);
			continue;
		}

		bool wait_for_child = true;

		// If ends with ampersand, don't wait child
		if (command[strlen(command) - 1] == '&')
		{
			wait_for_child = false;
			command[strlen(command) - 1] = '\0';
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

		// If waiting for child failed, exit
		if (wait_for_child && waitpid(child_pid, nullptr, 0) < 0)
			return -1;

		std::string history_entry = std::string(command) + "\t" + std::to_string(child_pid);
		history.add(history_entry.c_str());
	}

	return 0;
}