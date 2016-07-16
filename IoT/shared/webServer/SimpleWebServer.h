#include <ESP8266WiFi.h>

#define GET "GET"
#define POST "POST"

typedef struct Route {
  char* route;
  char* method;
  void (*response)(WiFiClient, int, char**, char**);

  Route() {};
  Route(char* route, char* method, void (*response)(WiFiClient, int, char**, char**)) {
    this->route = route;
    this->method = method;
    this->response = response;
  };
  ~Route() {
    delete [] route;
    delete [] method;
  }
};

class SimpleWebServer {
public:
  SimpleWebServer();
  void addRoute(Route*);
  void handleRequest();
private:
  bool started;
  Route** routes;
  int numRoutes;
  WiFiServer server;
};
