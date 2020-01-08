int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Error in arguments");
		return 0;
	}
	MainClient(argv[1],argv[2]);
}