//============================================================================
// Name        : OctaneCrawler.cpp
// Author      : Berlin Brown (berlin dot brown at gmail.com)
// Version     :
// Copyright   : Copyright Berlin Brown 2012-2013
// License     : BSD
// Description : This is the simplest possible web crawler in C++
//               Uses std_regex and std_algorithm
//============================================================================
 
#include <iostream>
#include <string>
#include <typeinfo>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <map> 

#include <regex>
#include <locale>
#include <ctype.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
 

#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;

using namespace std;
 

const int DELAY = 10000;
const std::string WRITE_DIR_PATH = "./";



graph_type *the_graph;
std::map<std::string, vertex_styled<_float_type> *> visited;
std::set<std::string> visited_hostname;

std::deque<std::pair<std::string, std::string> > worklist;


void to_lower(std::string &str)
{
    for (size_t i = 0; i < str.length(); i++) {
        str[i] = tolower(str[i]);
    }
}

std::wstring stringtowstring(const std::string& s)
{
    std::wstring temp(s.length(),L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp; 
}
 
class WebPage {
public:
    std::string hostname;
    std::string page;
 
    WebPage() {
        hostname = "";
        page = "";
    }
 
    std::string parseHttp(const std::string str) {
        const std::regex re("http://(.*)/?(.*)");
        std::smatch what;
        if (std::regex_match(str, what, re)) {
            std::string hst = what[1];
            to_lower(hst);
            return hst;
        }
        return "";
    }
 
    void parseHref(const std::string orig_host, const std::string str) {
        const std::regex re("http://(.*)/(.*)");
        std::smatch what;
        if (std::regex_match(str, what, re)) {
            // We found a full URL, parse out the 'hostname'
            // Then parse out the page
            hostname = what[1];
            to_lower(hostname);
            page = what[2];
        } else {
            // We could not find the 'page' but we can build the hostname
            hostname = orig_host;
            page = "";
        }
    }
 
    void parse(const std::string orig_host, const std::string hrf) {
        const std::string hst = parseHttp(hrf);
        if (!hst.empty()) {
            // If we have a HTTP prefix
            // We could end up with a 'hostname' and page
            parseHref(hst, hrf);
        } else {
            hostname = orig_host;
            page = hrf;
        }
        // hostname and page are constructed,
        // perform post analysis
        if (page.length() == 0) {
            page = "/";
        }
    }
};
 
std::string string_format(const char *fmt, ...) {
    int size = 255;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *) str.c_str(), size, fmt, ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
    return str;
}
 
 
int connect(void)
{
    std::string host, path;
    while (!worklist.empty()) {

        host = worklist.front().first;
        path = worklist.front().second;
        worklist.pop_front();

        auto cv = visited[host + path];
        assert(cv != nullptr);

        char cmd[4096];
        sprintf(cmd, "wget '%s%s' -t 1 -T 1 -O received", host.c_str(), path.c_str());
        system("rm -f received");
        system(cmd);

        std::ifstream t("received");
        std::string recv(
            (std::istreambuf_iterator<char>(t)),
            std::istreambuf_iterator<char>());

        // Parse the data //
        try {
            const std::regex rmv_all("[\\r|\\n]");
            const std::string s2 = std::regex_replace(recv, rmv_all, "");
            const std::string s = s2;
            const std::regex re("<a\\s+href\\s*=\\s*(\"([^\"]*)\")|('([^']*)')\\s*>");
            std::cmatch matches;
            // Using token iterator with sub-matches
            const int subs[] = { 2, 4 };
            std::sregex_token_iterator i(s.begin(), s.end(), re, subs);
            std::sregex_token_iterator j;
            
            color_type c = color_type(rand_range(0,1), rand_range(0,1), rand_range(0,1));
            for (; i != j; i++) {
                // Iterate through the listed HREFs and
                // move to next request //
                const std::string href = *i;
                if (href.length() != 0) {
                    WebPage* page = new WebPage();
                    page->parse(host, href);
                    const char* hrefc = page->page.c_str();
                    std::string new_path = string_format("/%s", hrefc);

                    if (strstr(new_path.c_str(), "javascript") ||
                        strstr(new_path.c_str(), "(") ||
                        strstr(new_path.c_str(), ")")) {
                        cout << "skip link : " << page->hostname << " page=" << hrefc  << endl;
                    }
                    else {
                        if (visited.find(page->hostname + new_path) == visited.end()) {
                            cout << "found link : " << page->hostname << " page=" << hrefc  << endl;
                            usleep(DELAY);

                            _float_type cx, cy, cz;
                            cv->x.coord(cx, cy, cz);
                            _float_type r = 5;
                            auto v = new vertex_styled<_float_type>(
                                rand_range(-r + cx, r + cx),
                                rand_range(-r + cy, r + cy),
                                rand_range(-r + cz, r + cz));

                            // v->font_family = "/Library/Fonts/Courier New.ttf";
                            // v->label = stringtowstring(new_path.substr(max((int)new_path.length() - 8, 0)));
                            // v->font_size = 24;
                            // v->size = 3;

                            std::string actual_hostname = page->hostname.substr(0, page->hostname.find_first_of("/"));
                            if (visited_hostname.find(actual_hostname) == visited_hostname.end()) {
                                visited_hostname.insert(actual_hostname);
                                v->font_family = "/Library/Fonts/Courier New.ttf";
                                v->label = stringtowstring(actual_hostname);
                                v->font_size = 24;
                                v->size = 5;
                            }
                            else {
                                v->size = 3;
                            }

                            v->color = c;
                            visited[page->hostname + new_path] = v;
                            the_graph->add_vertex(v);

                            auto e = new edge_styled<_float_type>(cv, v);
                            e->blendcolor = true;
                            the_graph->add_edge(e);

                            worklist.push_back(std::make_pair(page->hostname, new_path));
                        }
                        else {
                            auto v2 = visited[page->hostname + new_path];
                            auto e = new edge_styled<_float_type>(cv, v2);
                            e->strength = 0.01;
                            e->blendcolor = true;
                            the_graph->add_edge(e);
                        }
                    }
                    delete page;

                }
            }
        } catch (std::regex_error& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    return 1;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!the_graph) return;
    if (key == 'N' and action == GLFW_RELEASE) {
        new std::thread([&]() {
                connect();
            });
    } else {
        galaster_key_callback(window, key, scancode, action, mods);
    }
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s host path\n", argv[0]);
        return 0;
    }
    cout << "Launching program" << endl;
    GLFWwindow *window = galaster_init();
    if (window) {
        the_graph = new graph_type(1, 
            250,                    // f0
            0.02,                   // K
            0.001,                  // eps
            0.6,                    // damping
            1.2);                   // dilation
        std::string host = argv[1]; // "www.hrbcu.edu.cn";
        std::string path = argv[2]; // "/";
        auto v = new vertex_styled<_float_type>(0, 0, 0);
        v->font_family = "/Library/Fonts/Courier New.ttf";
        v->label = stringtowstring(host + path);
        v->font_size = 24;
        v->size = 5;
        v->color = color_type::green;
        visited[host + path] = v;
        the_graph->add_vertex(v);
        worklist.push_back(std::make_pair(host, path));
        visited_hostname.insert(host);

        glfwSetKeyCallback(window, key_callback);
        galaster_run(window, the_graph, 0.4);
    }

    cout << "Done" << endl;
    return 0;
}

