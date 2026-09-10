#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

int main()
{
	while (true)
	{
		std::string result;

		std::cout << "SamuelGauthier-DavidChalons< ";
		char command[128];
		std::cin.getline(command, 128);

		if (strlen(command) == 0)
			continue;

		std::vector<char*> args;
		char* prog = strtok(command, " ");
		char* tmp = prog;

		while (tmp != nullptr)
		{
			args.push_back(tmp);
			tmp = strtok(nullptr, " ");
		}

		const auto argv = new char*[args.size() + 1];

		for (int k = 0; k < args.size(); k++)
			argv[k] = args[k];

		argv[args.size()] = nullptr;

		if (strcmp(command, "exit") == 0)
			return 0;


		if (prog[0] == '.')
		{
			std::system(args[0]);
			continue;
		}

		pid_t kidpid = fork();

		if (kidpid < 0)
		{
			perror("Could not fork");
			return -1;
		}

		if (kidpid == 0)
		{
			execvp(prog, argv);
		}
		else if (waitpid(kidpid, nullptr, 0) < 0)
			return -1;
	}

	return 0;
}
