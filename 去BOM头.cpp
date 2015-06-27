/*
 *   test.cpp
 *   Copyright (C) 2015 陈伟琪，保留所有权利。
 */

/*
 *  ’test.cpp’ 包含了Windows平台下遍历目录的代码。
 */
 
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <regex>

#include <windows.h>

#using <System.dll>

using namespace System;
using namespace System::Text::RegularExpressions;
using namespace System::Runtime::InteropServices;

using namespace std;

void get_rid_of_bom( string );



int main( int argc, char *argv[] ){
	DWORD dwRet;
	BOOL boRet;
	char tmp_path[MAX_PATH];
	string local_path;
	
	system("cls");
	
	dwRet = GetCurrentDirectory( sizeof(tmp_path), tmp_path );
	local_path = argc==1?tmp_path:argv[1];
	cout<<"目标："<<local_path<<endl;
	cout<<"我正在辛勤地找出所有包含万恶的BOM头的文件，并把它们去掉的喵！"<<endl;
	
	list<string> recursion_stack;
	
	recursion_stack.push_front( local_path );
	
	while( !recursion_stack.empty() ){
		string current = recursion_stack.back();
		recursion_stack.pop_back();
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATA stFindData;
		hFind = FindFirstFile( current.c_str(), &stFindData );
		if( hFind==INVALID_HANDLE_VALUE ){
			cerr<<"无法打开文件：'"<<current<<"'。"<<endl;
			cerr<<"正在退出……"<<endl;
			return 1;
		}
		FindClose( hFind );
		if( FILE_ATTRIBUTE_DIRECTORY & stFindData.dwFileAttributes ){
			/*Directory*/
			//cout<<"目录：'"<<current<<"'。"<<endl;
			string target = current + "\\*";
			hFind = FindFirstFile( target.c_str(), &stFindData );
			if( hFind==INVALID_HANDLE_VALUE ){
				dwRet = GetLastError();
				if( dwRet==ERROR_NO_MORE_FILES ){
					/*Empty dir, just contine*/
					continue;
				}
				cerr<<"无法查看目录'"<<current<<"'的内容。"<<endl;
				cerr<<"错误代码："<<dwRet<<"。"<<endl;
				system("pause");
				return 2;
			}
			boRet = TRUE;
			while( boRet ){
				target = stFindData.cFileName;
				boRet = FindNextFile( hFind, &stFindData );
				if( target==".."||target=="." )
					continue;
				target = current+"\\"+target;
				recursion_stack.push_back( target );
				
			}
			dwRet = GetLastError();
			if( dwRet==ERROR_NO_MORE_FILES ){
				continue;
			}
			cerr<<"遍历目录'"<<current<<"'的过程中发生了错误。"<<endl;
			cerr<<"错误代码："<<dwRet<<"。"<<endl;
			system("pause");
			return 2;
		}else{
			/*Common file*/
			//cout<<"普通文件：'"<<current<<"'。"<<endl;
			get_rid_of_bom( current );
		}
	}
	cout<<"遍历完毕，按一下回车键或者直接关闭本窗口。"<<endl;
	system("pause");
	return 0;
}

void get_rid_of_bom( string file_path ){
	
	BOOL boRet;
	String^ match_str = ".*\\.txt";
	String^ file_str = Marshal::PtrToStringAnsi( (System::IntPtr)(char*)file_path.c_str() );
	
	boRet = Regex::IsMatch( file_str, match_str, RegexOptions::IgnoreCase );
	if( !boRet )
		return ;
	
	char buff[3];
	
	fstream fs;
	fs.open( file_path, fstream::in|fstream::binary );
	//if( fs.eof() )
		//return ;
	if( !fs.good() ){
		cerr<<"无法打开文件：'"<<file_path<<"'。"<<endl;
		return;
	}
	fs.read( buff, sizeof(buff) );
	if( memcmp( buff, "\xEF\xBB\xBF", sizeof(buff) ) )
		return ;
	
	cout<<"找到含有BOM头的txt文档：'"<<file_path<<"'！！"<<endl;

	int length;
	int length2;
	
	length = fs.tellg();
	fs.seekg( 0, fs.end );
	length2 = fs.tellg();
	
	length = length2 - length;
	
	if( length==0 )
		return;
	//cout<<"Length:"<<length<<endl;

	//char *buffer = new char[length];
	unique_ptr<char[]> buffer(new char [length]);
	fs.seekg( 3, fs.beg );
	fs.read( buffer.get(), length );
	if( !fs.good() ){
		cerr<<"已读取了"<< fs.gcount() <<"字节，但是不足"<<length<<"。放弃处理该文件。"<<endl;

		return ;
	}
	fs.close();
	fs.open( file_path, fstream::out|fstream::trunc|fstream::binary );
	fs.write( buffer.get(), length );
	fs.close();

}
