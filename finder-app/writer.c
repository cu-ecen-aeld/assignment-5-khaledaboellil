#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {

	if(argc!=3)
	{
		syslog(LOG_ERR, "Error: Please provide both the full path to the file and the text string.");
		return 1 ;
	}

	char * writefile = argv[1] ;
	char * writestr  = argv[2] ;

	if(! writefile || writefile[0]=='\0')
	{
		syslog(LOG_ERR, "Error: Please provide the full path to the file.");
        return 1 ;
	}
	if(! writestr|| writestr[0]=='\0')
	{
		syslog(LOG_ERR, "Error: Please provide the text string to be written to the file.");
        return 1 ;
	}

    // Write the content to the file
    FILE *file = fopen(writefile, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error: Failed to create the file '%s'.", writefile);
        return 1 ;
    }
    else
    {

	    fprintf(file, "%s", writestr);

	    fclose(file);

	    syslog(LOG_DEBUG, "Writing '%s' to '%s'", writestr, writefile);

		return 0 ;
	}
}
