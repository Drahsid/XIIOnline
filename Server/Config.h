#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

constexpr int cVERSION = 1;

enum Option {
    Version = 0,
    ServerPort,
    MaxConnections,
    Count
};

__forceinline double stringToDouble(std::string s)
{
    std::istringstream i(s);
    double x;
    if (!(i >> x))
        return 0;
    return x;
}

__forceinline bool stringToBool(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return (s == "1" || s == "on" || s == "true") || false;
}

struct ValueGetter {
    std::string inLine;

    int ReadInt() {
        printf("%s\n", inLine.c_str());
        std::string val = inLine.substr(inLine.find("=") + 1);
        return std::stoi(val.c_str());
    }

    float ReadFloat() {
        printf("%s\n", inLine.c_str());
        std::string val = inLine.substr(inLine.find("=") + 1);
        return std::stof(val.c_str());
    }

    double ReadDouble() {
        printf("%s\n", inLine.c_str());
        std::string val = inLine.substr(inLine.find("=") + 1);
        return stringToDouble(val);
    }

    bool ReadBool() {
        printf("%s\n", inLine.c_str());
        std::string val = inLine.substr(inLine.find("=") + 1);
        return stringToBool(val);
    }

    std::string ReadStr() {
        printf("%s\n", inLine.c_str());
        std::string val = inLine.substr(inLine.find("=") + 1);
        return val;
    }
};

struct ConfigOption {
    std::string name;
    std::string desc;
    ValueGetter getter;

    ConfigOption() {}
    ConfigOption(std::string n, std::string d) {
        name = n;
        desc = d;
    }
};

class UserConfig
{
public:
    int version = -1;
    int serverPort = 12400;
    int maxConnections = 16;

    ConfigOption options[Option::Count];

    UserConfig() {
        options[Option::Version] = ConfigOption("Version", "# File version. Changing this will cause the config to update.\n");
        options[Option::ServerPort] = ConfigOption("ServerPort", "# Port to use. [Default: 12400]\n");
        options[Option::MaxConnections] = ConfigOption("MaxConnections", "# Maximum number of connections. [Default: 16]\n");
    }

    #pragma warning (disable : 26812)
    std::string getVarByEnum(Option i) {
        switch (i)
        {
        case Version:
            return std::to_string(version);
            break;
        case ServerPort:
            return std::to_string(serverPort);
            break;
        case MaxConnections:
            return std::to_string(maxConnections);
            break;
        }
    }

    void UpdateConfigValues() {
        version = options[Option::Version].getter.ReadInt();
        serverPort = options[Option::ServerPort].getter.ReadInt();
        maxConnections = options[Option::MaxConnections].getter.ReadInt();
    }

    void NewUserConfig(std::fstream& config) {
        config.open("serverconfig.ini", std::fstream::trunc | std::fstream::out);
        config.close();
        config.open("serverconfig.ini", std::fstream::in | std::fstream::out);

        version = cVERSION;

        for (int i = 0; i < Option::Count; i++)
        {
            config << options[i].desc
                << options[i].name << "=" << getVarByEnum((Option)i) << "\n\n";
        }

        config.close();
    }

    void UpdateUserConfig()
    {
        std::fstream config;
        config.open("serverconfig.ini", std::fstream::in | std::fstream::out);
        if (!config)
        {
            printf("Config file not found! Writing a new one...\n\n");
            NewUserConfig(config);
        }
        else
        {
            std::string refLine;
            std::string inLine;
            while (std::getline(config, inLine))
            {
                for (int i = 0; i < Option::Count; i++)
                {
                    refLine = inLine.substr(0, inLine.find("="));
                    if (options[i].name == refLine) {
                        options[i].getter.inLine = inLine;
                    }
                }
                refLine.clear();
                inLine.clear();
            }
            config.close();

            UpdateConfigValues();

            if (version != cVERSION) NewUserConfig(config);
        }
    }
};

#endif // !CONFIG_H

