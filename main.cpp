#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
	while (true)
	{
		std::string result;

		std::cout << "VosNoms< ";
		char command[128];
		std::cin.getline(command, 128);

		if (strlen(command) != 0)
		{
			std::vector<char*> args;
			char* prog = strtok(command, " ");
			char* tmp = prog;

			while (tmp != NULL)
			{
				args.push_back(tmp);
				tmp = strtok(NULL, " ");
			}

			char** argv = new char*[args.size() + 1];

			for (int k = 0; k < args.size(); k++)
			{
				argv[k] = args[k];
			}

			argv[args.size()] = NULL;

			if (strcmp(command, "exit") == 0)
			{
				return 0;
			}


			if (prog[0] == '.')
			{
				std::system(args[0]);
			}
			else
			{
				pid_t kidpid = fork();

				if (kidpid < 0)
				{
					perror("Could not fork");
					return -1;
				}
				else if (kidpid == 0)
				{
					execvp(prog, argv);
				}
				else
				{
					if (waitpid(kidpid, 0, 0) < 0)
					{
						return -1;
					}
				}
			}
		}
	}

	return 0;
}
