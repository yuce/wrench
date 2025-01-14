/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

MIT Licence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#define STR_FILE_OPERATIONS
#include <wrench.h>

#include <stdio.h>
#include <assert.h>

#include "discrete_src/str.h"


int runTests( int number =0 );
void setup();

const char* g_errStrings[]=
{
	"ERR_None",

	"ERR_compiler_not_loaded",
	"ERR_function_hash_signature_not_found",
	"ERR_unknown_opcode",
	"ERR_unexpected_EOF",
	"ERR_unexpected_token",
	"ERR_bad_expression",
	"ERR_bad_label",
	"ERR_statement_expected",
	"ERR_unterminated_string_literal",
	"ERR_newline_in_string_literal",
	"ERR_bad_string_escape_sequence",
	"ERR_tried_to_load_non_resolvable",
	"ERR_break_keyword_not_in_looping_structure",
	"ERR_continue_keyword_not_in_looping_structure",
	"ERR_expected_while",
	"ERR_compiler_panic",
	"ERR_constant_refined",

	"ERR_run_must_be_called_by_itself_first",
	"ERR_hash_table_size_exceeded",
	"ERR_wrench_function_not_found",
	"ERR_array_must_be_indexed",
	"ERR_context_not_found",

	"ERR_usr_data_template_already_exists",
	"ERR_usr_data_already_exists",
	"ERR_usr_data_template_not_found",
	"ERR_usr_data_refernce_not_found",

	"ERR_bad_goto_label",
	"ERR_bad_goto_location",
	"ERR_goto_target_not_found",

	"ERR_switch_with_no_cases",
	"ERR_switch_case_or_default_expected",
	"ERR_switch_construction_error",
	"ERR_switch_bad_case_hash",
	"ERR_switch_duplicate_case",

	"warning_enums_follow",

	"WARN_c_function_not_found",
	"WARN_lib_function_not_found",
};

//------------------------------------------------------------------------------
void blobToHeader( WRstr const& blob, WRstr const& variableName, WRstr& header )
{
	header.format( "#ifndef _%s_H\n"
				   "#define _%s_H\n"
				   "\n"
				   "/******* wrench bytcode automatically generated header *******/\n\n"
				   "const int %s_bytecodeSize=%d;\n"
				   "const unsigned char %s_bytecode[]=\n"
				   "{\n",
				   variableName.c_str(),
				   variableName.c_str(),
				   variableName.c_str(),
				   (int)blob.size(),
				   variableName.c_str() );

	unsigned int p=0;
	while( p < blob.size() )
	{
		header += "\t";
		for( int i=0; i<16 && p<blob.size() ; i++ )
		{
			header.appendFormat( "0x%02X, ", (unsigned char)blob[p++] );
		}

		header.appendFormat( "// %d\n", p );
	}

	header += "};\n\n"
			  "#endif\n";
}

//------------------------------------------------------------------------------
void blobToAssemblyInc( WRstr const& blob, WRstr const& variableName, WRstr& header )
{
	header.format( "\n;******* wrench bytcode automatically generated include file *******\n\n"
				   ".export %s_bytecode\n"
				   "%s_bytecode: .export %s_bytecode\n",
				   variableName.c_str(),
				   variableName.c_str(),
				   variableName.c_str() );

	unsigned int p=0;
	while( p < blob.size() )
	{
		header += "\t.byte ";
		for( int i=0; i<16 && p<blob.size() ; i++ )
		{
			header.appendFormat( "$%02X, ", (unsigned char)blob[p++] );
		}

		header.shave( 2 );
		header.appendFormat( " ; %d\n", p );
	}

	header += "\n\n";
}

//------------------------------------------------------------------------------
const char* sourceOrder[]=
{
	"/utils.h",
	"/gc_object.h",
	"/vm.h",
	"/opcode.h",
	"/str.h",
	"/opcode_stream.h",
	"/cc.h",
	"/cc.cpp",
	"/vm.cpp",
	"/utils.cpp",
	"/operations.cpp",
	"/std.cpp",
	"/std_io.cpp",
	"/std_string.cpp",
	"/std_math.cpp",
	"/std_msg.cpp",
	""
};

//------------------------------------------------------------------------------
int usage()
{
	printf( "version: %d.%d\n"
			"usage: wrench <command> [options]\n"
			"where command is:\n"
			"\n"
			"v                              show version + help\n"
			"\n"
			"c [infile] [out as bytecode]   compile infile and output as raw bytecode\n"
			"\n"
			"ch [infile] [out file] [name]  compile infile and output as a header file\n"
			"                               of name \"out file\"\n"
			"                               suitable for '#include'\n"
			"                               the exported constants will be:\n"
			"                               const char* [name]_bytecode;\n"
			"                               const int [name]_bytecodeSize;\n"
			"ca [infile] [out file] [name]  compile infile and output as an inc file\n"
			"                               for assembly of name \"out file\"\n"
			"                               the exported constants will be:\n"
			"                               [name]_bytecode and\n"
			"                               [name]_bytecodeSize = $xxxx\n"
			"\n"
			"t                              run internal tests\n"
			"\n"
			"release [fromdir] [todir]      collect the discrete source files together\n"
			"                               and output [todir]/wrench.[cpp/h]\n"
			"\n"
			"rb [binary file to execute]    execute the file as if its bytecode\n"
			"r  [source file to execute]    compile and execute execute the file\n"
			"                               as if its source code\n"
			"\n", (int)WRENCH_VERSION_MAJOR, (int)WRENCH_VERSION_MINOR );

	return -1;
}

//------------------------------------------------------------------------------
static void printl( WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char outbuf[64000];
		argv[i].asString( outbuf, 64000 );
		printf( "%s", outbuf );
	}
	printf( "\n" );
}

//------------------------------------------------------------------------------
static void print( WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	for( int i=0; i<argn; ++i )
	{
		char outbuf[64000];
		argv[i].asString( outbuf, 64000 );
		printf( "%s", outbuf );
	}

	retVal.i = 20;
}

//------------------------------------------------------------------------------
int main( int argn, char* argv[] )
{
	assert( sizeof(WRValue) == sizeof(void*)*2 );
	assert( sizeof(float) == 4 );
	assert( sizeof(unsigned char) == 1 );

	if ( argn <= 1 )
	{
		return usage();
	}

	WRstr command(argv[1]);
	
	if ( command == "t" )
	{
//		setup();

		runTests( (argn >= 3) ? atoi(argv[2]) : 0 );
	}
	else if ( (command == "rb" || command == "r") && argn == 3 )
	{
		WRstr bytes;
		if ( command == "rb" )
		{
			if ( !bytes.fileToBuffer(argv[2]) )
			{
				printf( "Could not open bytecode [%s]\n", argv[2] );
				return usage();
			}
		}
		else
		{
			WRstr infile;
			if ( !infile.fileToBuffer(argv[2]) )
			{
				printf( "Could not open source file [%s]\n", argv[2] );
				return usage();
			}


			unsigned char* out;
			int outLen;

			int err = wr_compile( infile, infile.size(), &out, &outLen );
			if ( err )
			{
				printf( "compile error [%s]\n", g_errStrings[err] );
				return -1;
			}

			bytes.set( (char *)out, outLen );
			delete[] out;
		}

		WRState* w = wr_newState( 128 );
		wr_loadAllLibs(w);
		wr_registerFunction( w, "printl", printl );
		wr_registerFunction( w, "print", print );

		wr_run( w, (const unsigned char *)bytes.c_str(), bytes.size() );
		if ( wr_getLastError( w ) )
		{
			printf( "err: %d\n", (int)wr_getLastError(w) );
		}
	}
	else if ( command == "c" || command == "ch" || command == "ca" )
	{
		if ( argn < 4 )
		{
			return usage();
		}
		
		unsigned char* out;
		int outLen;

		WRstr code;
		if ( !code.fileToBuffer(argv[2]) )
		{
			printf( "Could not open [%s]\n", argv[2] );
			return usage();
		}

		int err = wr_compile( code, code.size(), &out, &outLen );
		if ( err )
		{
			printf( "compile error [%s]\n", g_errStrings[err] );
			return 0;
		}

		WRstr outname( argv[3] );
		if ( command == "c" && argn == 4)
		{
			code.set( (char *)out, outLen );
		}
		else if ( command == "ch" && argn == 5 )
		{
			blobToHeader( WRstr((char *)out, outLen), argv[4], code );
		}
		else if ( command == "ca" && argn == 5 )
		{
			blobToAssemblyInc( WRstr((char *)out, outLen), argv[4], code );
		}

		if ( !code.bufferToFile(outname) )
		{
			printf( "could not write to [%s]\n", outname.c_str() );
		}
		else
		{
			printf( "%s -> %s\n", argv[2], outname.c_str() );
		}

		delete[] out;
	}
	else if ( argn == 4 && WRstr(argv[1]) == "release" )
	{
		WRstr out = "#include \"wrench.h\"\n";
		WRstr name;		
		WRstr read;
		for( int s=0; sourceOrder[s][0]; ++s )
		{
			name = argv[2];
			name += sourceOrder[s];
			if ( !read.fileToBuffer(name) )
			{
				printf( "error reading [%s]\n", name.c_str() );
				return -1;
			}
			out += read;
		}

		name = argv[3];
		name += "/wrench.cpp";
		out.bufferToFile( name );

		name = argv[2];
		name += "/wrench.h";
		if ( !read.fileToBuffer(name) )
		{
			printf( "error reading [%s]\n", name.c_str() );
			return -1;
		}
		out = "#define WRENCH_COMBINED\n";
		out += read;
		name = argv[3];
		name += "/wrench.h";
		out.bufferToFile( name );
	}
	else
	{
		return usage();
	}

	return 0;
}

//------------------------------------------------------------------------------
static void emit( WRState* s, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	if ( argn >= 1 )
	{
		char buf[256];
		((WRstr*)usr)->appendFormat( "%s\n", argv->asString(buf, 256) );
	}
	retVal.i = 20;
}

//------------------------------------------------------------------------------
int runTests( int number )
{
	WRstr code;
	WRstr codeName;

	WRValue container;
	wr_makeContainer( &container );

	WRValue integer;
	wr_makeInt( &integer, 0 );
	wr_addValueToContainer( &container, "integer", &integer );

	char someArray[10] = "hello";
	wr_addArrayToContainer( &container, "name", someArray, 10 );

	char* someBigArray = new char[0x1FFFFF];
	someBigArray[0] = 10;
	someBigArray[10000] = 20;
	someBigArray[100000] = 30;
	someBigArray[0x1FFFFE] = 40;

	char byte = (char)0x99;
	char byte2 = (char)0x99;
	wr_addArrayToContainer( &container, "b", &byte, 1 );
	wr_addArrayToContainer( &container, "c", &byte2, 1 );
		
	wr_addArrayToContainer( &container, "big", someBigArray, 0x1FFFFF );

	FILE* tfile = fopen( "test_files.txt", "r" );
	char buf[256];
	int fileNumber = 0;
	int err = 0;

	WRState* w = wr_newState( 128 );

	wr_loadAllLibs( w );




	while( fgets(buf, 255, tfile) && (err==0) )
	{
		if ( !number || (number == fileNumber) )
		{
			WRstr expect;
			codeName = buf;
			codeName.trim();

			if ( code.fileToBuffer(codeName) )
			{
				unsigned int t = 0;

				for( ; (t < code.size()) && (code[t] != '~'); ++t );

				t += 2;

				for( ; t<code.size() && (code[t] != '~'); ++t )
				{
					expect += code[t];
				}

				printf( "test [%d][%s]: ", fileNumber, codeName.c_str() );

				unsigned char* out;
				int outLen;

				err = wr_compile( code, code.size(), &out, &outLen );
				if ( err )
				{
					printf( "compile error [%s]\n", g_errStrings[err] );
					return -1;
				}

				WRstr logger;
				wr_registerFunction( w, "print", emit, &logger );

				WRValue* V = wr_returnValueFromLastCall(w);

				wr_destroyContext( 0 ); // test that this works

				WRContext* context = 0;
				wr_destroyContext( context );
				context = wr_run( w, out, outLen );

				if ( !wr_getLastError(w) )
				{
					integer.i = 2456;
					someArray[1] = 'e';

					wr_callFunction( w, context, "userCheck", &container, 1 );
					V = wr_returnValueFromLastCall( w );
					if ( V && V->i == 77 )
					{
						assert( integer.i == 56789 );
						assert( someArray[1] == 'c' );
					}

					unsigned char testString[12] = "test string";
					WRValue val;
					wr_makeString( context, &val, testString, 11 );
					wr_callFunction( w, context, "stringCheck", &val, 1 );
					wr_freeString( &val );
				}

				if ( err )
				{
					printf( "execute error [%d]\n", err );
				}
				else if ( logger != expect )
				{
					printf( "error: expected\n"
							"-----------------------------\n"
							"%s\n"
							"saw:-------------------------\n"
							"%s"
							"\n"
							"-----------------------------\n",
							expect.c_str(), logger.c_str() );
				}
				else
				{
					printf( "PASS\n" );
				}

				delete[] out;
				wr_destroyContext( context );
			}
			else if ( fileNumber != 0 )
			{
				printf( "test [%d][%s]: FILE NOT FOUND\n", fileNumber, codeName.c_str() );
			}
		}
		
		fileNumber++;
	}

	wr_destroyContainer( &container );

	wr_destroyState( w );

	delete[] someBigArray;
	fclose( tfile );
	return err;
}


// COMPLETE EXAMPLE MUST WORK


#include "wrench.h"

void log2( WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr )
{
	char buf[512];
	for( int i=0; i<argn; ++i )
	{
		printf( "%s", argv[i].asString(buf, 512) );
	}
}

const char* wrenchCode = 

						"print( \"Hello World!\\n\" ); "
						"for( i=0; i<10; i++ )       "
						"{                           "
						"    log( i );               "
						"}                           ";


void setup()
{
//	Serial.begin( 115200 );
//	delay( 2000 ); // wait for link to come up for sample

	WRState* w = wr_newState(); // create the state

	wr_registerFunction( w, "print", print ); // bind a function

	unsigned char* outBytes; // compiled code is alloc'ed
	int outLen;

	int err = wr_compile( wrenchCode, (int)strlen(wrenchCode), &outBytes, &outLen );
	if ( err == 0 )
	{
		wr_run( w, outBytes, outLen ); // load and run the code!
		delete[] outBytes; // clean up 
	}

	wr_destroyState( w );
}

void loop()
{}