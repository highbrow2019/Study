/* Copy Constructor : eliminate the implicit invocation of copy contructor.
 *
 */


#include <cstdio> // from stdio.h in C
#include <cassert>

class LogFile 
{
	public:
		LogFile( const char *fname)
		{
			filePtr = fopen( fname, "wt");
		}

		~LogFile()
		{
			if( filePtr != nullptr)
			{
				fclose( filePtr);
			}
		}

		void write( const char *txt )
		{
			assert( filePtr != nullptr);
			fputs( txt, filePtr );
		}
	private:
		FILE *filePtr;
};

int main()
{
	LogFile d1( "test.log" );

	d1.write("1st line\n");

	LogFile d2( "assertTest.txt" );
	d2.write("asserFile read only"); // assert here <-- as the file assertTest.txt is changed to read only externally 

	return 0;
}
