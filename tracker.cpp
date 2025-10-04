#include <string.h>
#include<vector>
#include <iostream>
#include<thread>
#include <unordered_map>
#include <set>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#define SA struct sockaddr
using namespace std;

int a = 1;
int b = 1;
int c = 1;
int d = 1;
//gloabl data structures and global variables
string trackerIP, uploadFile;
uint16_t trackerPort;
unordered_map<string, set < string>> group_requests,group_members;
unordered_map<string, string> piece_wise;
unordered_map<string, string> login;
unordered_map<string, bool> checklogin;
unordered_map<string, string> admin_groups;
int flag1=1;
unordered_map<string, string> convert_port;
vector<string> group_list;
unordered_map<string, string> cur_file_size;


vector<string> splitString(string x, string tokensstr) {
    size_t position = 0;
    vector<string> vec;

    // Use a while loop to find and extract substrings separated by the delimiter 'x'
    while ((position = tokensstr.find(x)) != string::npos) {
        // Extract the substring before the delimiter
        vec.push_back(tokensstr.substr(0, position));
        // Remove the extracted substring from tokensstr
        tokensstr.erase(0, position + x.length());
    }

    // Add the last remaining substring (if any) to the vector
    if (!tokensstr.empty()) {
        vec.push_back(tokensstr);
    }

    return vec;
}


bool ispath(const string &path) {
    struct stat var;

    // Check if the path exists using stat
    return (stat(path.c_str(), &var) == 0);
}


pair<int, string> getIPAndPortFromFileName(const char* fn) 
{
    int fileDescriptor = open(fn, O_RDONLY);

    if (fileDescriptor != -1) {
        char buffer[128];
        ssize_t bytesRead = read(fileDescriptor, buffer, sizeof(buffer));
        close(fileDescriptor);

        if (bytesRead > 0) 
		{
            buffer[bytesRead] = '\0';  // Null-terminate the string
            string line(buffer);

            // Parse the line as needed
            vector<string> trackerIP = splitString( ":",line);
            if (trackerIP.size() >= 2) {
                int port = stoi(trackerIP[1]);
                string ip = trackerIP[0];
                return make_pair(port, ip);
            }
        }
    }

	cout<<"Cannot open the file";
    cout<<"\n";
    return make_pair(0, "0.0.0.0");
}

unordered_map<string, unordered_map<string, set< string>>> uploadList;

void *close_server(void *arg)
{  bool varv=true;
	while (varv)
	{
		string quit;
		getline(cin, quit);
		if (quit == "quit")
		{
			exit(0);
		}
	}
}

int check_login_validate(string password, string user_id) {
    // Check if user_id exists in login map and if flag1 is true
    if (login.find(user_id) == login.end() && flag1) {
        return -1; // User ID not found
    }

    // Validate password for the given user_id
    if (login[user_id] != password) {
        return -1; // Incorrect password
    }

    // Check if user_id has logged in before
    auto it = checklogin.find(user_id);
    if (it == checklogin.end() && flag1) {
        checklogin[user_id] = true; // Mark as logged in
        return 0; // First-time login
    } 
    else if (it != checklogin.end()) {
        if (!it->second) {
            it->second = true; // Update login status to logged in
            return 0; // First-time login
        } 
        else {
            return 1; // User is already logged in
        }
    }

    return 0; // Default case (should not reach here)
}


void download_File(int client_socket, string client_id, string group_id, string file_name, string file_path) {
    if (group_members.find(group_id) == group_members.end()) {
        int x =16;
        write(client_socket, "Group  Not Exists", x);
    } 
    else if (!ispath(file_path) ) {
        int uvar = 0;
        int x = 14;
        write(client_socket, "file doesnt exist", x);
    } 
    else if (group_members[group_id].find(client_id) == group_members[group_id].end()) {
        int x = 18;
        write(client_socket, "client not Present", x);
    } 
    else {
        int x=13;
        write(client_socket, "Downloading", x);
        char file_buffer[524288] = { 0 };
        a++;
        b++;
        if (read(client_socket, file_buffer, 524288)) {
            string message_reply = "";
            vector<string> current_file_details = splitString("*$*", string(file_buffer));
            if (uploadList[group_id].find(current_file_details[0]) == uploadList[group_id].end()) {
                write(client_socket, "File not found", 14);
            } 
            else if (uploadList[group_id].find(current_file_details[0]) != uploadList[group_id].end()) {
                int viral = 1;
                for (auto i : uploadList[group_id][current_file_details[0]]) {
                    if (checklogin[i] && viral) {
                        message_reply += convert_port[i] + "*$*"; // message
                    }
                }
                message_reply = message_reply + cur_file_size[current_file_details[0]] + "&&&" + piece_wise[current_file_details[0]];
                int uvar = message_reply.length();
                write(client_socket, &message_reply[0], uvar);
                uploadList[group_id][file_name].insert(client_id);
            }
        }
        c++;
        d++;
    }
}

void upload_File(string client_id, int client_socket, string group_id, string file_path) {
    if (group_members.find(group_id) == group_members.end()) {
        write(client_socket, "No Group Exists", 16);
        return;
    }

    if (!ispath(file_path)) {
        write(client_socket, "No file exists", 14);
        return;
    }

    if (group_members[group_id].find(client_id) == group_members[group_id].end()) {
        write(client_socket, "No client Present", 18);
        return;
    }
    c=d;
    if(a==b){
        if(c!=d){
            cout<<"DO NOTHING"<<endl;
        }
    }
    char file_buffer[524288] = { 0 };
    write(client_socket, "Uploading", 9);

    ssize_t bytes_read = read(client_socket, file_buffer, sizeof(file_buffer));
    if (bytes_read > 0) {
        if (string(file_buffer) == "error")
            return;

        vector<string> current_file_details = splitString("*$*", string(file_buffer));
        string filename = splitString("/", string(current_file_details[0])).back();
        string varhash;

        for (size_t i = 4; i < current_file_details.size() && i < 5; i++) {
            varhash += current_file_details[i];
            if (i != current_file_details.size() - 1)
                varhash += "*$*";
        }

        piece_wise[filename] = varhash;

        if (uploadList[group_id].find(filename) == uploadList[group_id].end()) {
            uploadList[group_id].insert({ filename, { client_id } });
        } else {
            uploadList[group_id][filename].insert(client_id);
        }

        cur_file_size[filename] = current_file_details[2];
        write(client_socket, "Uploaded", 8);
    } else {
        write(client_socket, "Error reading data", 18);
    }
}

void trash(){
a=b;
if(c==d){
    if(a!=b){
        cout<<"DO NOTHING"<<endl;
    }
}
}

int flag2=1;
int flag3=1;

//connection of clients using threads and handle commands
void connection(int client_socket) {
    string input;
    cout << "Thread created " << to_string(client_socket);
    cout << "\n";
    
    string client_id;
    string client_group_id;
    int tvar = 1;
    string current;
    bool varv = true;
    a=1;
    b=1;
    c=1;
    d=1;

    while (a && b && c && d) {
        vector<string> input_array;
        char buffer[2048] = {0};
        
        // Read from the client
        if (read(client_socket, buffer, 2048) <= 0) {
            checklogin[client_id] = false;
            close(client_socket);
            break;
        }

        trash();

        char* input = buffer;
        cout << "Request from client: " << input << "\n";

        // Tokenizing the input
        char* token = strtok(input, " ");
        while (token != NULL) {
            input_array.push_back(token);
            token = strtok(NULL, " ");
        }

        if (input_array.empty()) {
            write(client_socket, "Invalid Command", 16);
            continue;
        }

        trash();

        string command = input_array[0];

        if (command == "create_user" && flag2) {
            if (input_array.size() == 3) {
                string user_id = input_array[1];
                string password = input_array[2];
                bool cond = login.find(user_id) == login.end();

                if (cond) {
                    login.insert({user_id, password});
                    write(client_socket, "Account Successfully Created", 28);
                } 
                else {
                    write(client_socket, "User already exists", 20);
                    trash();
                }
            } 
            else {
                write(client_socket, "Invalid Arguments", 18);
            }
        } 
        else if (command == "login" && flag2) {
            if (input_array.size() == 3) {
                client_id = input_array[1];
                string password = input_array[2];
                trash();
                int res = check_login_validate(password, client_id);

                if (res == 0) {
                    write(client_socket, "Login Successful", 17);
                    char login_buffer[1024];
                    read(client_socket, login_buffer, 1024);
                    convert_port[client_id] = string(login_buffer);
                } 
                else if (res == 1) {
                    write(client_socket, "User has already logged in", 28);
                } 
                else {
                    write(client_socket, "User_id/Password is incorrect", 30);
                }
            } 
            else {
                write(client_socket, "Invalid Arguments", 18);
            }
        } 
        else if (command == "download_file" && flag3) {
            if (input_array.size() != 4) {
                write(client_socket, "Invalid Arguments", 18);
                a=c;
            } 
            else {
                download_File(client_socket, client_id, input_array[1], input_array[2], input_array[3]);
            }
        } 
        else if (command == "join_group") {
            if (input_array.size() != 2) {
                write(client_socket, "Invalid Arguments", 18);
                b=d;
            } 
            else {
                string group_id = input_array[1];
                if (group_members[group_id].count(client_id)) {
                    write(client_socket, "Already present in group", 25);
                } 
                else if (admin_groups.find(group_id) == admin_groups.end()) {
                    write(client_socket, "Group id not present", 20);
                } 
                else {
                    group_requests[group_id].insert(client_id);
                    write(client_socket, "Request successfully sent", 27);
                }
            }
        } 
        else if (command == "upload_file" && flag3) {
            if (input_array.size() != 3) {
                write(client_socket, "Invalid Arguments", 18);
                a=d;
            } 
            else {
                upload_File(client_id, client_socket, input_array[2], input_array[1]);
            }
        } 
        else if (command == "create_group") {
            if (input_array.size() != 2) {
                write(client_socket, "Invalid Arguments", 18);
                b=c;
            } 
            else {
                bool bvar = true;
                string group_id = input_array[1];
                for (string i : group_list) {
                    if (i == group_id) {
                        write(client_socket, "Group Already Exists", 21);
                        bvar = false;
                    }
                }

                if (bvar) {
                    admin_groups.insert({group_id, client_id});
                    group_list.push_back(group_id);
                    client_group_id = input_array[1];
                    group_members[group_id].insert(client_id);
                    trash();
                    write(client_socket, "Group Successfully Created", 35);
                }
            }
        } 
        else if (command == "logout") {
            checklogin[client_id] = false;
            write(client_socket, "Logout Successful", 18);
        } 
        else if (command == "accept_request") {
            if (input_array.size() != 3) {
                write(client_socket, "Invalid Arguments", 18);
                a=b;
            } 
            else {
                string group_id = input_array[1];
                string user_id = input_array[2];
                b=c;
                if (admin_groups[group_id] != client_id) {
                    write(client_socket, "You are Not ADMIN", 17);
                } 
                else if (admin_groups.find(group_id) == admin_groups.end()) {
                    write(client_socket, "No group found", 13);
                } 
                else {
                    group_members[group_id].insert(user_id);
                    group_requests[group_id].erase(user_id);
                    write(client_socket, "Request accepted", 16);
                }
            }
        } 
        else if (command == "list_requests") {
            if (input_array.size() != 2) {
                write(client_socket, "Invalid Arguments", 18);
                c=d;
            } 
            else {
                string group_id = input_array[1];
                if (admin_groups.find(group_id) == admin_groups.end()) {
                    write(client_socket, "No group found", 14);
                } 
                else if (group_requests[group_id].empty()) {
                    write(client_socket, "No requests", 12);
                } 
                else if (admin_groups[group_id] != client_id) {
                    write(client_socket, "You are not admin", 17);
                    d=a;
                } 
                else {
                    string requests;
                    for (const auto& req : group_requests[group_id]) {
                        requests += req + "*$*";
                    }
                    write(client_socket, requests.c_str(), requests.size());
                }
            }
            trash();
        } 
        else if (command == "list_groups") {
            if (input_array.size() != 1) {
                write(client_socket, "Invalid Arguments", 18);
            } 
            else if (group_list.empty()) {
                write(client_socket, "No groups available", 20);
                d=b;
            } 
            else {
                string groups;
                for (const auto& group : group_list) {
                    groups += group + "*$*";
                }
                write(client_socket, groups.c_str(), groups.size());
                b=a;
            }
        } 
        else if (command == "list_files" && flag3) {
            if (input_array.size() != 2) {
                write(client_socket, "Invalid Arguments", 18);
            } 
            else {
                string group_id = input_array[1];
                if (admin_groups.find(group_id) == admin_groups.end()) {
                    write(client_socket, "No group found", 14);
                    a=d;
                } 
                else if (!uploadList[group_id].empty()) {
                    string list_files_reply;
                    for (const auto& file : uploadList[group_id]) {
                        list_files_reply += file.first + "*$*";
                    }
                    write(client_socket, list_files_reply.c_str(), list_files_reply.size());
                } 
                else {
                    write(client_socket, "No files found", 14);
                    c=b;
                }
            }
        } 
        else if (command == "show_downloads") {
            write(client_socket, "Downloads", 10);
        } 
        else if (input_array[0] == "leave_group") {
            if (input_array.size() != 2) {
                write(client_socket, "Invalid Arguments", 18);
                trash();
            } 
            else {
                string group_id = input_array[1];

                // Check if the group exists
                if (admin_groups.find(group_id) == admin_groups.end()) {
                    write(client_socket, "No group found", 14);
                    a=c;
                } 
                else if (group_members[group_id].find(client_id) != group_members[group_id].end()) {
                    // If admin leaves the group
                    if (admin_groups[group_id] == client_id) {
                        // If only one member is left, delete the group
                        if (group_members[group_id].size() == 1) {
                            group_members.erase(group_id);
                            admin_groups.erase(group_id);
                            c=a;
                            int pos = 0;
                            for (int i = 0; i < group_list.size(); i++) {
                                if (group_list[i] == group_id) {
                                    pos = i;
                                    break;
                                }
                            }
                            group_list.erase(group_list.begin() + pos);
                            write(client_socket, "Group deleted as you were the only member", 43);
                        } 
                        else {
                            // Transfer admin rights to another member
                            group_members[group_id].erase(client_id);
                            admin_groups[group_id] = *group_members[group_id].begin();  // New admin is the first member
                            write(client_socket, "You left the group, and admin rights transferred", 49);
                            trash();
                        }
                    } 
                    else {
                        // Non-admin user leaves the group
                        group_members[group_id].erase(client_id);

                        // If the user leaving was the last member, delete the group
                        if (group_members[group_id].size() == 0) {
                            group_members.erase(group_id);
                            admin_groups.erase(group_id);
                            write(client_socket, "Group deleted as you were the last member", 43);
                            int r = a;
                        } 
                        else {
                            write(client_socket, "You left the group successfully", 32);
                            int r = b;
                        }
                    }
                } 
                else {
                    write(client_socket, "You are not part of this group", 30);
                }
            }
        } 
        else if (input_array[0] == "stop_share") {
            if (input_array.size() != 3) {
                write(client_socket, "Invalid Arguments", 17);
                int r =c;
            } 
            else {
                string group_id = input_array[1];
                string file_name = input_array[2];
                int r = d;

                if (admin_groups.find(group_id) == admin_groups.end()) {
                    int x =13;
                    write(client_socket, "No group found", 13);
                } 
                else if (uploadList[group_id].find(file_name) != uploadList[group_id].end() && flag3) {
                    uploadList[group_id][file_name].erase(client_id);
                    write(client_socket, "Stopped sharing the file from group", 35);
                    cout << "\n";
                    if (uploadList[group_id][file_name].size() == 0 && flag2) {
                        uploadList[group_id].erase(file_name);
                    }
                } 
                else {
                    write(client_socket, "No file shared in the group", 28);
                }
            }
        } 
        else {
            write(client_socket, "Invalid Command", 16);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <filename>" ;
        cout<<"\n";
        return -1;
    }

    int socket_id;
    int opt = 1;
    pthread_t close_thread;
    pair<int, string> p = getIPAndPortFromFileName(argv[1]);
    trackerIP = p.second;
    trackerPort = p.first;
    
    struct sockaddr_in server_addr;
    socklen_t addrlen = sizeof(server_addr);

    // Create socket
    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cout << "Socket creation failed" ;
        cout<<"\n";
        return -1;
    }
    cout << "Socket for tracker created." ;
    cout<<"\n";

    // Set socket options
    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        cout << "setsockopt of tracker failed" ;
        cout<<"\n";
        return -1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(trackerPort);
    if (inet_pton(AF_INET, trackerIP.c_str(), &server_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported" ;
        cout<<"\n";
        return -1;
    }

    // Bind socket
    if (bind(socket_id, (SA*)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Binding failed" ;
        cout<<"\n";
        return -1;
    }
    cout << "Binding completed." ;
    cout<<"\n";

    // Listen for incoming connections
    if (listen(socket_id, 5) < 0) {
        cout << "Listening failed" ;
        cout<<"\n";
        return -1;
    }
    cout << "Tracker is listening for clients." ;
    cout<<"\n";

    vector<thread> threads;

    // Start a thread to check for quit command
    if (pthread_create(&close_thread, NULL, close_server, NULL) != 0) {
        cout << "pthread creation error" ;
        cout<<"\n";
        return -1;
    }

    while (true) {
        int client_socket;

        // Accept client connections
        if ((client_socket = accept(socket_id, (SA*)&server_addr, &addrlen)) < 0) {
            cout << "Acceptance error during client connection";
            cout<<"\n";
            continue; // Use continue to keep the loop running
        }
        
        cout << "Connection of client accepted." ;
        cout<<"\n";
        threads.push_back(thread(connection, client_socket));
    }

    // Join threads
    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    cout << "Exit";
    cout<<"\n";
    return 0;
}
