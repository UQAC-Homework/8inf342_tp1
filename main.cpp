#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

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
	char* command_history[5] = {};

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
			for (const auto current : command_history)
			{
				if (current == nullptr)
					break;

				std::cout << current << std::endl;
			}

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

		// Record command in history
		bool was_set = false;

		for (auto& i : command_history)
		{
			if (i != nullptr)
				continue;

			i = copy_command;
			was_set = true;
			break;
		}

		if (!was_set)
		{
			constexpr auto size = std::size(command_history);

			for (int i = 0; i < size - 1; i++)
				command_history[i] = command_history[i + 1];

			command_history[size - 1] = copy_command;
		}
	}

	return 0;
}
