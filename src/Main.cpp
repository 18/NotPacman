// Copyright 2018 Krystian Stasiowski

#include "windows.h"
#include <iostream>
#include <string>
#include <vector>
#include "Console.h"
#include "Command.h"
#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include "xxhash.hpp"
#include <urlmon.h>
#include <tuple>
#include <chrono>
#pragma comment(lib, "urlmon.lib")

void Help(std::vector<std::string> args);
void Cd(std::vector<std::string> args);
void Dl(std::vector<std::string> args);
void Clear(std::vector<std::string> args);
void Exit(std::vector<std::string> args);
void Dbg(std::vector<std::string> args);
void Ls(std::vector<std::string> args);
void Generate(std::vector<std::string> args);

const std::vector<Command> COMMANDS = { Command("help", CommandType::STANDARD, Help), Command("cd", CommandType::STANDARD, Cd), Command("dl",CommandType::STANDARD, Dl), Command("clear", CommandType::STANDARD, Clear), Command("exit", CommandType::STANDARD, Exit), Command("dbg", CommandType::STANDARD, Dbg), Command("ls", CommandType::DEBUG, Ls), Command("generate", CommandType::DEBUG, Generate) };
std::string directory;
bool debug = false;

std::vector<std::string> SplitString(std::string str, const char delim)
{
  std::vector<std::string> output;
  std::string current;
  int pos;
  while ((pos = str.find(delim)) != std::string::npos)
  {
    current = str.substr(0, pos);
    output.push_back(current);
    str.erase(0, pos + 1);
  }
  output.push_back(str);
  return output;
}

std::vector<std::string> GetAllFiles(const std::string& path)
{
  std::vector<std::string> output;
  for (std::experimental::filesystem::recursive_directory_iterator i(path), end; i != end; ++i)
    if (!is_directory(i->path()))
      output.push_back(i->path().string());
  return output;
}

unsigned long long GetFileHash(const std::string& path)
{
  std::ifstream ifs(path, std::ios::binary | std::ios::ate);
  if (!ifs)
  {
    Console::WriteLine("[ ERROR ] hash failed", Color::LIGHT_RED);
    return 0;
  }
  std::ifstream::pos_type size = ifs.tellg() > 277872640 ? 277872640 : static_cast<int>(ifs.tellg());
  char* buffer = new char[size];
  ifs.seekg(0, std::ios::beg);
  ifs.read(buffer, size);
  unsigned long long hash = xxh::xxhash<64>(reinterpret_cast<const void*>(buffer), static_cast<size_t>(size));
  delete[] buffer;
  ifs.close();
  return hash;
}

std::vector<std::string> GetAllLines(const std::string& url)
{
  std::string path = directory + "repo.txt";
  URLDownloadToFileA(NULL, url.c_str(),path.c_str(), NULL, NULL);
  std::vector<std::string> output;
  std::ifstream ifs(path);
  if (!ifs)
  {
    Console::WriteLine("[ ERROR ] host not found", Color::LIGHT_RED);
    return std::vector<std::string>();
  }
  std::string line;
  while (std::getline(ifs, line))
  {
    output.push_back(line);
  }
  ifs.close();
  remove(path.c_str());
  return output;
}

std::string ParseBytesToSize(unsigned long long bytes)
{
  std::string str;
  std::string unit;
  if (bytes < 1000000)
  {
    unit = "KB";
    str = std::to_string(bytes / 1000.0f);
  }
  else if (bytes < 1000000000)
  {
    unit = "MB";
    str = std::to_string(bytes / 1000000.0f);
  }
  else if (bytes < 1000000000000)
  {
    unit = "GB";
    str = std::to_string(bytes / 1000000000.0f);
  }
  else
  {
    unit = "TB";
    str = std::to_string(bytes / 1000000000000.0f);
  }
  return str.substr(0, str.find('.') + 3) + " " + unit;
}

void main()
{
  Console::SetTitle("notpacman");
  Console::Resize(1000, 700);
  Console::WriteLine("");
  Console::WriteLineCentered("[ not pacman ]", Color::LIGHT_AQUA);
  Console::WriteLineCentered("created by cry", Color::LIGHT_AQUA);
  Console::WriteListCentered(
    {
        "                                                                      ..;===+.",
        "                                                                .:=iiiiii=+=  ",
        "                                                             .=i))=;::+)i=+,  ",
        "                                                          ,=i);)I)))I):=i=;   ",
        "                                                       .=i==))))ii)))I:i++    ",
        "                                                     +)+))iiiiiiii))I=i+:'    ",
        "                                .,:;;++++++;:,.       )iii+:::;iii))+i='      ",
        "                             .:;++=iiiiiiiiii=++;.    =::,,,:::=i));=+'       ",
        "                           ,;+==ii)))))))))))ii==+;,      ,,,:=i))+=:         ",
        "                         ,;+=ii))))))IIIIII))))ii===;.    ,,:=i)=i+           ",
        "                        ;+=ii)))IIIIITIIIIII))))iiii=+,   ,:=));=,            ",
        "                      ,+=i))IIIIIITTTTTITIIIIII)))I)i=+,,:+i)=i+              ",
        "                     ,+i))IIIIIITTTTTTTTTTTTI))IIII))i=::i))i='               ",
        "                    ,=i))IIIIITLLTTTTTTTTTTIITTTTIII)+;+i)+i`                 ",
        "                    =i))IIITTLTLTTTTTTTTTIITTLLTTTII+:i)ii:'                  ",
        "                   +i))IITTTLLLTTTTTTTTTTTTLLLTTTT+:i)))=,                    ",
        "                   =))ITTTTTTTTTTTLTTTTTTLLLLLLTi:=)IIiii;                    ",
        "                  .i)IIITTTTTTTTLTTTITLLLLLLLT);=)I)))))i;                    ",
        "                  :))IIITTTTTLTTTTTTLLHLLLLL);=)II)IIIIi=:                    ",
        "                  :i)IIITTTTTTTTTLLLHLLHLL)+=)II)ITTTI)i=                     ",
        "                  .i)IIITTTTITTLLLHHLLLL);=)II)ITTTTII)i+                     ",
        "                  =i)IIIIIITTLLLLLLHLL=:i)II)TTTTTTIII)i'                     ",
        "                +i)i)))IITTLLLLLLLLT=:i)II)TTTTLTTIII)i;                      ",
        "              +ii)i:)IITTLLTLLLLT=;+i)I)ITTTTLTTTII))i;                       ",
        "             =;)i=:,=)ITTTTLTTI=:i))I)TTTLLLTTTTTII)i;                        ",
        "           +i)ii::,  +)IIITI+:+i)I))TTTTLLTTTTTII))=,                         ",
        "         :=;)i=:,,    ,i++::i))I)ITTTTTTTTTTIIII)=+'                          ",
        "       .+ii)i=::,,   ,,::=i)))iIITTTTTTTTIIIII)=+                             ",
        "      ,==)ii=;:,,,,:::=ii)i)iIIIITIIITIIII))i+:'                              ",
        "     +=:))i==;:::;=iii)+)=  `:i)))IIIII)ii+'                                  ",
        "   .+=:))iiiiiiii)))+ii;                                                      ",
        "  .+=;))iiiiii)));ii+                                                         ",
        " .+=i:)))))))=+ii+                                                            ",
        ".;==i+::::=)i=;                                                               ",
        ",+==iiiiii+,                                                                  ",
        "`+=+++;`                                                                      "
    }, Color::LIGHT_PURPLE);
  Console::SetColor(Color::GREEN);
  Sleep(1500);
  Console::Clear();
  while (true)
  {
    std::vector<std::string> command = SplitString(Console::ReadLinePrompt(directory + ">"), ' ');
    if (command[0].empty())
    {
      continue;
    }
    std::vector<Command>::const_iterator it = std::find_if(COMMANDS.begin(), COMMANDS.end(), [&command](const Command& c) { return c.Name() == command[0]; });
    if (it != COMMANDS.end())
    {
      if (!debug && (*it).Type() != CommandType::STANDARD)
      {
        Console::WriteLine("[ ERROR ] command not found", Color::LIGHT_RED);
      }
      else
      {
        std::vector<std::string> args(command.begin() + 1, command.end());
        (*it).Execute(args);
      }
    }
    else
    {
      Console::WriteLine("[ ERROR ] command not found", Color::LIGHT_RED);
    }
  }
  std::cin.ignore();
}

void Help(std::vector<std::string> args)
{
  if (!args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  Console::WriteList({ "[ INFO ] help - show this dialouge", "[ INFO ] dl <repo url> - download all files in a repository", "[ INFO ] cd <dir> - set the directory to download to", "[ INFO ] clear - clear the screen", "[ INFO ] generate - generate hashes for files in directory", "[ INFO ] exit - close the program" }, Color::LIGHT_AQUA);
}

void Dl(std::vector<std::string> args)
{
  if (args.size() != 1)
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  if (directory.empty())
  {
    Console::WriteLine("[ ERROR ] invalid directory", Color::LIGHT_RED);
    return;
  }
  std::vector<std::string> lines = GetAllLines(args[0]);
  if (lines.empty())
  {
    Console::WriteLine("[ ERROR ] mirror not found", Color::LIGHT_RED);
    return;
  }
  if (lines[0] != "[REPO]")
  {
    Console::WriteLine("[ ERROR ] invalid repository file", Color::LIGHT_RED);
    return;
  }
  std::string url = lines[1];
  std::vector<std::tuple<unsigned long long, std::string, unsigned long long>> files;
  unsigned long long total_size = 0;
  for (std::vector<std::string>::iterator it = lines.begin() + 2; it != lines.end(); ++it)
  {
    std::string& str = *it;
    size_t pos1 = str.find_first_of('*');
    size_t pos2 = str.find_last_of('*');
    unsigned long size = std::stoull(str.substr(pos2 + 1));
    total_size += size;
    files.emplace_back(std::stoull(str.substr(pos1 + 1, pos2 - pos1)), str.substr(0, pos1), size);
  }
  std::string confirm = Console::ReadLinePrompt("Download " + std::to_string(files.size()) + " files (" + ParseBytesToSize(total_size) + ")? [Y/N]: ");
  std::locale l("");
  if (confirm.length() > 1)
  {
    Console::WriteLine("[ OK ] canceling download", Color::LIGHT_AQUA);
    return;
  }
  else if (std::tolower(confirm[0], l) == 'n')
  {
    Console::WriteLine("[ OK ] canceling download", Color::LIGHT_AQUA);
    return;
  }
  int downloaded = 0;
  int hash_mismatch = 0;
  int deleted = 0;
  for (std::string& s : GetAllFiles(directory))
  {
    unsigned long long hash = GetFileHash(s);
    s.erase(0, directory.length());
    if (find_if(files.begin(), files.end(), [&s](const std::tuple<unsigned long long, std::string, unsigned long long>& p) { return std::get<1>(p) == s; }) == files.end())
    {
      ++deleted;
      remove(std::string(directory +  "\\" + s).c_str());
    }
  }
  COORD starting_pos = Console::GetCursorPosition();
  Console::ShowConsoleCursor(false);
  std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
  std::string loading_bar = "----------";
  for (int i = 0; i < files.size(); ++i)
  {
    float percent = (static_cast<float>((i + 1)) / files.size()) * 100;
    int bar_progress = static_cast<int>(std::round(percent / 10));
    if (loading_bar[bar_progress] != '#')
    {
      for (int j = 0; j < bar_progress; j++)
      {
        loading_bar[j] = '#';
      }
    }
    Console::SetCursorPosition(0, starting_pos.Y);
    Console::Write("downloading files\t[" + loading_bar + "] " + std::to_string(static_cast<int>(percent)) + "%");
    const std::tuple<unsigned long long, std::string, unsigned long long>& file = files[i];
    std::string url_path = std::get<1>(file);
    for (char& c : url_path)
    {
      if (c == '\\')
        c = '/';
    }
    std::string path = directory + std::get<1>(file);
    std::string url_comp = url + url_path;
    if (exists(std::experimental::filesystem::v1::path(path)))
    {
      if (GetFileHash(path) == std::get<0>(file))
      {
        continue;
      }
    }
    ++downloaded;
    std::experimental::filesystem::v1::create_directories(std::experimental::filesystem::v1::path(path).remove_filename().string().c_str());
    URLDownloadToFileA(NULL, url_comp.c_str(), path.c_str(), NULL, NULL);
    if (GetFileHash(path) != std::get<0>(file))
    {
      ++hash_mismatch;
    }
  }
  Console::WriteLine("");
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  std::string time = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0f);
  Console::WriteLine("[ SUCESS ] downloaded " + std::to_string(downloaded) + " file(s) in " + time.substr(0, time.find('.') + 3) + "s", Color::LIGHT_GREEN);
  if (hash_mismatch)
  {
    Console::WriteLine("[ WARNING ] " + std::to_string(hash_mismatch) + " hash mismatch(s)", Color::LIGHT_YELLOW);
  }
  if (deleted)
  {
    Console::WriteLine("[ WARNING ] " + std::to_string(deleted) + " file(s) deleted", Color::LIGHT_YELLOW);
  }
  Console::ShowConsoleCursor(true);
}

void Cd(std::vector<std::string> args)
{
  if (args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  std::string dir;
  for (const std::string& s : args)
    dir += (s + " ");
  if (dir.find("~") != std::string::npos)
    dir = std::experimental::filesystem::current_path().string() + std::string(dir.begin() + 1, dir.end());
  if (!exists(std::experimental::filesystem::v1::path(dir)) || dir.length() <= 2)
  {
    Console::WriteLine("[ ERROR ] invalid directory", Color::LIGHT_RED);
    return;
  }
  std::locale l("");
  bool contains_alpha = false;
  for (char c : dir)
  {
    if (std::isalnum(c, l))
    {
      contains_alpha = true;
      break;
    }
  }
  if (!contains_alpha)
  {
    Console::WriteLine("[ ERROR ] invalid directory", Color::LIGHT_RED);
    return;
  }
  for (char& c : dir)
    if (c == '/')
      c = '\\';
  while (dir[dir.length() - 1] == ' ')
  {
    dir.erase(dir.end() - 1);
  }
  while (dir[dir.length() - 1] == '\\')
  {
    dir.erase(dir.end() - 1);
  }
  directory = dir + "\\";
}

void Clear(std::vector<std::string> args)
{
  if (!args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  Console::Clear();
}

void Exit(std::vector<std::string> args)
{
  if (!args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  exit(0);
}

void Dbg(std::vector<std::string> args)
{
  if (!args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  debug = !debug;
  if (debug)
  {
    Console::WriteLine("[ OK ] debug mode enabled", Color::LIGHT_GREEN);
  }
  else
  {
    Console::Write("[ OK ] debug mode ", Color::LIGHT_GREEN);
    Console::WriteLine("disabled", Color::LIGHT_RED);
  }
}

void Ls(std::vector<std::string> args)
{
  if (!args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  if (directory.empty())
  {
    Console::WriteLine("[ ERROR ] invalid directory", Color::LIGHT_RED);
    return;
  }
  for (const std::string& s : GetAllFiles(directory))
  {
    Console::WriteLine("[ FILE ] " + s.substr(s.find_last_of("\\") + 1) + " [ HASH ] " + std::to_string(GetFileHash(s)), Color::LIGHT_AQUA);
  }
}

void Generate(std::vector<std::string> args)
{
  if (!args.empty())
  {
    Console::WriteLine("[ ERROR ] invalid arguments", Color::LIGHT_RED);
    return;
  }
  if (directory.empty())
  {
    Console::WriteLine("[ ERROR ] invalid directory", Color::LIGHT_RED);
    return;
  }
  std::string name = "hashes" + std::to_string(time(0)) + ".txt";
  std::ofstream output(directory + name);
  for (std::string& s : GetAllFiles(directory))
  {
    unsigned long long hash = GetFileHash(s);
    std::ifstream ifs(s, std::ifstream::ate | std::ifstream::binary);
    std::ifstream::pos_type size = ifs.tellg();
    ifs.close();
    s.erase(0, directory.length());
    if (s == name)
    {
      continue;
    }
    output << s << "*" << std::to_string(hash) << "*" << size << std::endl;
  }
  Console::WriteLine("[ SUCESS ] generated hashes ", Color::LIGHT_GREEN);
}