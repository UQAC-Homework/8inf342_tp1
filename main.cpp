#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

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

		// Split arguments
		std::vector<char*> args;
		char* first_keyword = strtok(command, " ");
		char* current_keyword = first_keyword;

		while (current_keyword != nullptr)
		{
			args.push_back(current_keyword);
			current_keyword = strtok(nullptr, " ");
		}

		const auto argv = new char*[args.size() + 1];

		for (int k = 0; k < args.size(); k++)
			argv[k] = args[k];

		argv[args.size()] = nullptr;

		// 
		if (first_keyword[0] == '.')
		{
			std::system(args[0]);
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
			execvp(first_keyword, argv);
		}
		else
		{
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

		delete[] argv;
	}

	return 0;
}
