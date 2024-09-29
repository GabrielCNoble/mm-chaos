#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	if(argc > 1)
	{
		char vert_list_name_buffer[1024];
		uint32_t vert_list_name_cursor = 0;
		uint32_t input_cursor = 0;
		FILE *file = fopen(argv[1], "w");

		fprintf(file, "SCENE_CMD_ROOM_VERT_LIST_LIST(&%sVertListList)\n\n", argv[2]);

		fprintf(file, "RoomVertList %sVertList[] = {\n", argv[2]);
		do
		{
			vert_list_name_cursor = 0;
			while(argv[3][input_cursor] != ' ' && argv[3][input_cursor] != '\0')
			{
				vert_list_name_buffer[vert_list_name_cursor] = argv[3][input_cursor];
				input_cursor++;
				vert_list_name_cursor++;
			}

			while(argv[3][input_cursor] == ' ')
			{
				input_cursor++;
			}

			vert_list_name_buffer[vert_list_name_cursor] = '\0';
			
			if(vert_list_name_cursor > 0)
			{
				fprintf(file, "	ROOM_VERT_LIST(%s),\n", vert_list_name_buffer);
			}
		}
		while(vert_list_name_cursor > 0);
	
		fprintf(file, "};\n\n");
		fprintf(file, "RoomVertListList %sVertListList = {\n", argv[2]);
		fprintf(file, "	%sVertList, ARRAY_COUNT(%sVertList)\n", argv[2], argv[2]);
		fprintf(file, "};\n\n");
		fprintf(file, "extern RoomVertListList %sVertListList;\n", argv[2]);
		fclose(file);
	}
	return 0;
}
