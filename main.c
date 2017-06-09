//  This file is part of RAX.
//
//  RAX is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  RAX is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with RAX.  If not, see <http://www.gnu.org/licenses/>.


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

enum num_repr
{
	dec,
	hex,
	oct
};

//Convert a character to lowercase
char ch_lower(char x)
{
	if (x>='A' && x<='Z')
	{
		return x+32;
	}
	else
	{
		return x;
	}
}

//Case insensitive string equality test
bool lax_streq(char *a, char *b)
{
 	while (*a!='\0' && *b!='\0')
	{
		if (ch_lower(*a)!=ch_lower(*b))
		{
			return false;
		}
		a++;
		b++;
	}
	return true;
}

bool streq(char *a, char *b)
{
	return strcmp(a,b)==0;
}

void skip_whitespace(char **str)
{
	for (;**str==' '||**str=='\t'||**str=='\r'||**str=='\n'||**str=='\v'||**str=='\f';(*str)++){}
}

int main(int argc, char *argv[])
{
	enum num_repr cur_mode=dec;
	long int cur_count;
	long int new_val;
	char *end;
	cur_count=0;
	//A 64-bit integer in octal will take 19 characters including the '0' prefix and '\n\0'
	//This is the largest input we will take
	#define MAX_INPUT 19
	static char buffer[MAX_INPUT];
	static char *out_of_range_msg="\033[31m[ Error ]\033[00m Value out of range\n";
	static char *invalid_syntax_msg="\033[31m[ Error ]\033[00m Invalid syntax, type '?' for help\n";

	for (size_t i=1;i<argc;i++)
	{
		if (streq(argv[i],"-h") || streq(argv[i],"--help"))
		{
			puts("Usage: rax [OPTION]... [VALUE]\n"
				"A program designed to facilitate the counting of numbers\n"
				"\n"
				"-x, --hex Display count in hexadecimal\n"
				"-o, --oct Display count in octal\n");
				exit(0);
		}
		else if (streq(argv[i],"-x") || streq(argv[i],"--hex"))
		{
			cur_mode=hex;
		}
		else if (streq(argv[i],"-o") || streq(argv[i],"--oct"))
		{
			cur_mode=oct;
		}
		else
		{
			cur_count=strtol(argv[i],&end,0);
			if (*end!='\0')
			{
				fputs("\033[31m[ Error ]\033[00m Invalid initial value specified\n",stderr);
				exit(EDOM);
			}
			else if (errno==ERANGE)
			{
				fputs("\033[31m[ Error ]\033[00m Initial value out of range\n",stderr);
				exit(EDOM);
			}
		}
	}

	while (true)
	{
		//Display the counter differently depending on the users preference
		switch (cur_mode)
		{
			case hex:
				printf("[0x%lx] ",cur_count);
				break;
			case oct:
				printf("[0%lo] ",cur_count);
				break;
			default:
				printf("[%li] ",cur_count);
				break;
		}

		if (fgets(buffer,MAX_INPUT,stdin)==NULL)
		{
			fputs("\033[31m[ Error ]\033[00m Can't get input from terminal\n",stderr);
			exit(EIO);
		}

		end=buffer;

		switch (buffer[0])
		{
			case 'd':
			case 'D':
				if (buffer[1]=='\n' || lax_streq(buffer+1,"dec\n") || lax_streq(buffer+1,"ecimal\n") || lax_streq(buffer+1,"enary\n"))
				{
					cur_mode=dec;
				}
				else
				{
						fputs(invalid_syntax_msg,stderr);
				}
				break;

			case 'o':
			case 'O':
				if (buffer[1]=='\n' || lax_streq(buffer+1,"ct\n") || lax_streq(buffer+1,"ctal\n"))
				{
					cur_mode=oct;
				}
				else
				{
						fputs(invalid_syntax_msg,stderr);
				}
				break;

			case 'x':
			case 'X':
				if (buffer[1]=='\n')
				{
					cur_mode=hex;
				}
				else
				{
						fputs(invalid_syntax_msg,stderr);
				}
				break;

			case '$':
				if (buffer[1]=='\n') //Check if only '+' has been passed
				{
					cur_count=0;
				}
				else
				{
					end++;
					new_val=strtol(end,&end,0);
					if (*end!='\n')
					{
						fputs(invalid_syntax_msg,stderr);
					}
					else if (errno==ERANGE)
					{
						fputs(out_of_range_msg,stderr);
					}
					else
					{
						cur_count=new_val;
					}
				}
				break;

			case '+':
				if (buffer[1]=='\n') //Check if only '+' has been passed
				{
					cur_count++;
				}
				else
				{
					end++;
					new_val=strtol(end,&end,0);
					if (*end!='\n')
					{
						fputs(invalid_syntax_msg,stderr);
					}
					else if (errno==ERANGE)
					{
						fputs(out_of_range_msg,stderr);
					}
					else
					{
						cur_count+=new_val;
					}
				}
				break;

			case '-':
				if (buffer[1]=='\n') //Check if only '-' has been passed
				{
					cur_count--;
				}
				else
				{
					end++;
					new_val=strtol(end,&end,0);
					if (*end!='\n') //Check if the whole string was converted
					{
						fputs(invalid_syntax_msg,stderr);
					}
					else if (errno==ERANGE)
					{
						fputs(out_of_range_msg,stderr);
					}
					else
					{
						cur_count-=new_val;
					}
				}
				break;

			case 'h':
			case 'H':
					if (lax_streq(buffer+1,"ex\n")  || lax_streq(buffer+1,"hexadecimal\n"))
					{
						cur_mode=hex;
						break;
					}
					else if (buffer[1]!='\n' && !lax_streq(buffer+1,"elp\n"))
					{
						fputs(invalid_syntax_msg,stderr);
						break;
					}

			case '?':
				puts("[ Help ]\n"
					"$\n"
					"\tSets the counter to 0\n"
					"\n"
					"+\n"
					"\tIncrements the counter by 1\n"
					"\n"
					"-\n"
					"\tDecrements the counter by 1\n"
					"\n"
					"\n"
					"$ <INTEGER>\n"
					"\tSets the counter to the specified integer\n"
					"\n"
					"+ <INTEGER>\n"
					"\tIncrements the counter by the given integer\n"
					"\n"
					"- <INTEGER>\n"
					"\tDecrements the counter by the given integer\n"
					"\n"
					"\n"
					"x, hex, hexadecimal\n"
					"\tDisplays counter in hexadecimal\n"
					"\n"
					"d, dec, denary, decimal\n"
					"\tDisplays counter in denary\n"
					"\n"
					"o, oct, octal\n"
					"\tDisplays counter in octal\n"
					"\n"
					"?, h, help\n"
					"\tShows help\n"
					"\n"
					"q, quit, e, exit\n"
					"\tClose the program\n"
					"\n"
					"\n"
					"All integers can be prepended with '0x' for hexadecimal, '0' for octal and '-' for a negative number\n"
					"All commands are case insensitive\n"
				);
				break;
			case '\n':
				break;

			case 'E':
			case 'e':
				if (buffer[1]=='\n' || lax_streq(buffer+1,"xit\n"))
				{
					exit(0);
				}
			case 'Q':
			case 'q':
				if (buffer[1]=='\n' || lax_streq(buffer+1,"uit\n"))
				{
					exit(0);
				}
			default:
				fputs(invalid_syntax_msg,stderr);
				break;


		}
	}
}
