#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <map>
#include <sstream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define PORT 7777
#define PRINTNUSERS if (nclients)\
printf ("%d user online \n", nclients);\
else printf ("No users online\n");
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
using namespace std;
SOCKET Connection;
int nclients = 0;
HOSTENT* hst;
string username, password;

void SendMsg(string msg, SOCKET socket)//Функция отправки сообщений
{
	int msg_size = msg.size();
	send(socket, (char*)&msg_size, sizeof(int), NULL);
	send(socket, msg.c_str(), msg_size, NULL);
}

string RecvMsg(SOCKET socket)//Функция приема строковых сообщений
{
	int msg_size = 0;
	recv(socket, (char*)&msg_size, sizeof(int), NULL);
	char* msg = new char[msg_size + 1];
	msg[msg_size] = '\0';
	recv(socket, msg, msg_size, NULL);
	string str_msg = msg;
	delete[] msg;
	return str_msg;
}

int RecvInt(SOCKET socket) {//Функция приема целочисленных сообщений
	int value = 0;
	recv(socket, (char*)&value, sizeof(int), NULL);
	return value;
}

bool checkLogin(string login, string password, string filename) {
	ifstream file(filename);
	if (file.is_open()) {
		std::string line;
		while (getline(file, line)) {
			// Извлекаем логин и пароль из текущей строки
			istringstream iss(line);
			string savedLogin, savedPassword;
			iss >> savedLogin >> savedPassword;
			// Проверяем, совпадает ли логин и пароль с заданными
			if (login == savedLogin && password == savedPassword) {
				file.close();
				return true;
			}
		}
		file.close();
	}
	return false;
}

int Size(string Name)
{
	fstream File(Name);//Открытие файла
	int SizeFile;
	File.seekg(0, ios::end);//Перемещение метки в конец файла
	SizeFile = File.tellg();//Получение размера файла
	File.close();
	return SizeFile;
}

void HandleNumber(int number) {
	// Открываем файл output.txt в текстовом режиме для записи
	std::ofstream file("C:\\auth\\output.txt");
	if (!file) {
		std::cerr << "Ошибка открытия файла!" << std::endl;
		return;
	}

	// Записываем число в файл
	file << number;

	// Закрываем файл
	file.close();
}

void SendFile(SOCKET socket) {
	string Name = RecvMsg(socket);
	string Extension = RecvMsg(socket);
	string FullNameFile = "D:\\Send\\" + Name + "." + Extension;
	ifstream File(FullNameFile, ios::in | ios::binary | ios::ate); // Открываем файл для чтения в бинарном режиме

	if (File.is_open()) {
		// Получаем текущее время
		time_t rawtime;
		struct tm* timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char buffer[80];
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
		string timeStr(buffer);



		// Сохраняем информацию о файле в файле "D:\\Raz\\naz.txt"
		ofstream InfoFile("D:\\Raz\\naz.txt", ios::app);
		if (InfoFile.is_open()) {
			InfoFile << "Name: " << Name << ", Extension: " << Extension << ", Data&Time: " << timeStr << endl;
			InfoFile.close();
		}
		else {
			cerr << "Unable to open 'D:\\Raz\\naz.txt' for writing." << endl;
		}

		SendMsg("File_found", socket);
		int SizeFile = File.tellg(); // Получение размера файла
		send(socket, (char*)&SizeFile, sizeof(int), NULL); // Отправка размера файла

		File.seekg(0, ios_base::beg); // Перемещение метки в начало файла

		const int SizeBuffer = 1024; // Размер буфера
		char TransferBuff[SizeBuffer]; // Массив для хранения данных из файла

		cout << "Запрошенный файл на передачу: " << Name << endl;

		// Логирование размера файла в файл "D:\\Raz\\raz.txt"
		ofstream LogFile;
		LogFile.open("D:\\Raz\\raz.txt", ios::app);
		LogFile << FullNameFile << ": " << SizeFile << " bytes" << ",		 Data&Time: " << timeStr << endl;
		LogFile.close(); // Закрытие файла для логирования

		do {
			int SendSize = min(SizeFile, SizeBuffer);
			File.read(TransferBuff, SendSize); // Чтение из файла данных

			send(socket, (char*)&SendSize, sizeof(int), NULL); // Отправка размера буфера
			send(socket, TransferBuff, SendSize, NULL); // Отправка данных клиенту

			SizeFile -= SendSize;
		} while (SizeFile > 0);

		File.close();
		cout << "Файл " << Name << " успешно отправлен" << endl;
	}
	else {
		SendMsg("File_not_found", socket);
	}
}

DWORD WINAPI ServiceToClient(LPVOID client_socket)// Эта функция создается в отдельном потоке и обсуживает очередного подключившегося клиента независимо от остальных
{
	SOCKET my_socket;// Создание новой переменной типа SOCKET
	my_socket = ((SOCKET*)client_socket)[0]; // Присваиваем my_socket значение client_socket
	int SRecv = 0;
	std::ifstream file("C:\\auth\\output.txt");
	if (!file) {
		std::cerr << "Ошибка открытия файла!" << std::endl;
		return 1;
	}
	// Считываем переменную NFiles из файла
	int NFiles;
	file >> NFiles;
	// Закрываем файл
	file.close();
	string StrRecv;
	do {
		recv(my_socket, (char*)&SRecv, sizeof(int), 0);
		char* CharRecv = new char[SRecv + 1]; //Выделение динамической памяти под название файла
		CharRecv[SRecv] = '\0';
		recv(my_socket, CharRecv, SRecv, 0);
		StrRecv = CharRecv;
		if (NFiles != 0) {
			SendMsg("File_sent", my_socket);
			if (StrRecv == "Recv") {
				SendFile(my_socket);
				NFiles--;
			}
		}
		else {
			SendMsg("File_sending_error", my_socket);
			cout << "Ошибка: Клиент превысил количество передаваемых файлов!" << endl;
		}
	} while (StrRecv != "Exit");
	nclients--;
	PRINTNUSERS;
	return 0;
}

int main(int argc, const char* argv[]) {
	setlocale(LC_ALL, "");
	cout << "SERVER (Nagodkin 20PT1)" << endl;
	WSAData wsaData;//Инициализация Библиотеки Сокетов
	WORD DDLVersions = MAKEWORD(2, 1); // Установка версии библиотеки
	WSAStartup(DDLVersions, &wsaData);// 1-ый параметр - версия; 2-ой параметр - ссылка на структуру
	WSAData;
	if (WSAStartup(DDLVersions, &wsaData) != 0)// Проверка подключения библиотеки
	{
		cout << "Error WSAStartup" << endl;
		return 0;
	}
	if ((Connection = socket(AF_INET, SOCK_STREAM, NULL)) < 0)// Если значения сокета установлены
	{
		WSACleanup();//Деиницилизация библиотек иWinsock
		cout << "Error socket" << endl;
		return 0;
	}
	SOCKADDR_IN address;//Связывание сокета с локальным адресом
	int sizeOFaddress = sizeof(address);//Размер адреса
	address.sin_port = htons(PORT);//Записываем номер порта
	address.sin_family = AF_INET;// Записываем идентификатор домена
	address.sin_addr.s_addr = 0;//не забываем о сетевом порядке, IP адрес хоста
	if (bind(Connection, (sockaddr*)&address, sizeof(address)))//вызываем bind для связывания
	{
		closesocket(Connection);//закрываем сокет
		WSACleanup(); //Деиницилизация библиотек иWinsock
		cout << "Error bind" << endl;
		return 0;
	}
	if (listen(Connection, 100))//ожидание подключений.
	{
		closesocket(Connection);//закрываем сокет
		WSACleanup();//Деиницилизация библиотек иWinsock
		cout << "Error listen" << endl;
		return 0;
	}
	cout << "Ожидание подключений" << endl;
	sockaddr_in client_addr;
	SOCKET client_socket{};
	int client_addr_size = sizeof(client_addr);
	while ((client_socket = accept(Connection, (sockaddr*)&client_addr, &client_addr_size)))//цикл извлечения запросов на подключение из очереди
	{
		send(client_socket, (char*)&nclients, sizeof(int), NULL);
		string login = RecvMsg(client_socket);
		string password = RecvMsg(client_socket);
		string filename = "C:\\auth\\logins.txt";
		if (checkLogin(login, password, filename)) {
			cout << "Аутентификация прошла успешно" << endl;
			SendMsg("Authentication_was_successful", client_socket);
		}
		else {
			cout << "Аутентификация не удалась" << endl;
			SendMsg("Authentication_was_not_successful_the_login_or_password_were_incorrect", client_socket);
			nclients--;
			//PRINTNUSERS;
		}
		int NFiles;
		if (login == "admin" && password == "admin") {
			int number = RecvInt(client_socket);
			HandleNumber(number);
		}
		if (nclients < 100) {// Если количество подключеных клиентов меньше 1, то
			nclients++;//увеличиваем счетчик подключившихся клиентов
			hst = gethostbyaddr((char*)&client_addr.sin_addr.s_addr, 4, AF_INET);// Получение хоста
			printf("пользователь: %s [ IP-адрес: %s ]\n", (hst) ? hst->h_name : "",
				inet_ntoa(client_addr.sin_addr));//вывод сведений о клиенте
			PRINTNUSERS;// Вывод сведений о количестве активных клиентов
			DWORD thID;// Вызов нового потока для обслуживания клиента
			LPVOID client_Socket;

			CreateThread(NULL, NULL, ServiceToClient, &client_socket, NULL, &thID);// Создание отдельного потока для выполнения функции ServiceToClient
		}
		else {
			closesocket(client_socket);// закрываем сокет
			continue;
		}
	}
	return 0;
}