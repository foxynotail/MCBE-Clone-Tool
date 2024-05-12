#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#define UNUSED(x) (void)(x)

using namespace std;

bool sortFunc(const string& a, const string& b) {

	//cout << "A: " << a << " B: " << b << std::endl;

	int Ax = a[0] | (a[1] << 8) | (a[2] << 16) | (a[3] << 24);
	int Az = a[4] | (a[5] << 8) | (a[6] << 16) | (a[7] << 24);
	int Bx = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
	int Bz = b[4] | (b[5] << 8) | (b[6] << 16) | (b[7] << 24);

	if (Ax != Bx) {
		return (Ax < Bx);
	}

	return (Az < Bz);
}

bool file_exists(const std::string& name) {
	ifstream f(name.c_str());
	return f.good();
}

// trim from start

std::string ltrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {
		return !std::isspace(c);
		}));
	return s;
}

// trim from end
std::string rtrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {
		return !std::isspace(c);
		}).base(), s.end());
	return s;
}

// trim from both ends
std::string trim(std::string s) {
	return ltrim(rtrim(s));
}


bool check_number(std::string str) {

	// remove starting dash if minus number
	if (str.substr(0, 1) == "-") {
		str = str.substr(1);
	}
	for (int i = 0; i < str.length(); i++) {
		if (isdigit(str[i]) == false) {
			return false;
		}
	}
	return true;
}

std::ofstream create_log() {

	// DATE & TIME
	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::time_t start_time = std::chrono::system_clock::to_time_t(now);
	char timedisplay[100];
	char dateTimeDisplay[100];
	struct tm buf;
	errno_t err = localtime_s(&buf, &start_time);
	std::string dateTime;
	if (std::strftime(timedisplay, sizeof(timedisplay), "%Y-%m-%d-%H:%M:%S", &buf)) {
		dateTime = timedisplay;
	}
	std::string dateFile;
	if (std::strftime(dateTimeDisplay, sizeof(dateTimeDisplay), "%Y-%m-%d %H-%M-%S", &buf)) {
		dateFile = dateTimeDisplay;
		std::string dateFileName;
		int i = 0;
		bool fileExists = true;
		while (fileExists == true) {
			i++;
			std::string iStr = std::to_string(i);
			dateFileName = "logs/Clone " + dateFile + " [" + iStr + "].txt";
			if (!file_exists(dateFileName)) {
				fileExists = false;
			}
		}
		dateFile = dateFileName;
	}

	std::ofstream logFile(dateFile, ios::out);
	return logFile;
}

void rLog(std::string string, std::ofstream& logFile) {
	std::cout << string << std::endl;
	logFile << string << std::endl;
}


void int_to_bytes(int32_t x, std::string& y)
{
	int32_t byteArray[4];
	byteArray[0] = (uint8_t)(x >> 0);
	byteArray[1] = (uint8_t)(x >> 8);
	byteArray[2] = (uint8_t)(x >> 16);
	byteArray[3] = (uint8_t)(x >> 24);

	for (int i : byteArray) {
		y.push_back(i);
	}
}

void int_to_byte(int32_t x, std::string& y)
{
	int32_t byteArray[1];
	byteArray[0] = (uint8_t)(x >> 0);

	for (int i : byteArray) {
		y.push_back(i);
	}
}

void int_to_char_bytes(int32_t x, unsigned char* byteArray)
{
	byteArray[0] = (uint8_t)(x >> 0);
	byteArray[1] = (uint8_t)(x >> 8);
	byteArray[2] = (uint8_t)(x >> 16);
	byteArray[3] = (uint8_t)(x >> 24);
}

std::string int_to_hex(int i)
{
	std::stringstream stream;
	stream << std::hex << i;
	return stream.str();
}

std::string float_to_bytestring(float v) {

	unsigned char const* p = reinterpret_cast<unsigned char const*>(&v);
	std::stringstream s;
	for (std::size_t i = 0; i != sizeof(float); ++i) {
		s << p[i];
	}
	return s.str();
}

std::string int_to_bytestring(int v) {

	unsigned char const* p = reinterpret_cast<unsigned char const*>(&v);
	std::stringstream s;
	for (std::size_t i = 0; i != sizeof(int); ++i) {
		s << p[i];
	}
	return s.str();
}


i_act bytestring_to_int(leveldb::Slice slice, int offset) {

	union {
		i_act num;
		uint8_t bytes[8];
	} retval = { 0 };

	for (std::size_t i = 0; i < 8; ++i) {
		retval.bytes[i] = (uint8_t)slice[offset + 8 - i - 1];
	}
	return retval.num;
}
#endif