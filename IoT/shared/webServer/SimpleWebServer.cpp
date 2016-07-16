#include <SimpleWebServer.h>

SimpleWebServer::SimpleWebServer() : server(80) {
  started = false;
  numRoutes = 0;
}

void SimpleWebServer::addRoute(Route* r) {

  Route** newRoutes = new Route*[numRoutes + 1];
  Serial.println(numRoutes + 1);
  for (int i = 0; i < numRoutes; i++) {
    newRoutes[i] = routes[i];
  }
  Serial.println("Finished Copying routes");
  newRoutes[numRoutes] = r;
  Serial.println("Added new Route");
  if (numRoutes++ > 0) {
    Serial.println("Deleting old routes");
    delete [] routes;
  }
  Serial.println("Reassigning routes");
  routes = newRoutes;
  Serial.println("done");
}

int parseParams(char* params, char**& keys, char**& values) {

  int numParams = 0;
  int idx = 0;
  while(params[idx] != 0) {
    // Serial.print("char: ");
    // Serial.print(params[idx]);
    // Serial.println(params[idx] == '=');
    if (params[idx] == '+') {
      params[idx] = ' ';
    }
    numParams += params[idx++] == '=';
    // Serial.print("Num Params: ");
    // Serial.println(numParams);
  }

  Serial.print("total params found: ");
  Serial.println(numParams);

  keys = new char*[numParams];
  values = new char*[numParams];

  idx = 0;

  char* kvPair = strtok(params, "&");
  while(kvPair != 0) {
    char* colon = strchr(kvPair, '=');
    *colon = 0; // set the equals sign to 0 to split the string
    keys[idx] = kvPair;
    // Serial.print("Found key ");
    // Serial.println(kvPair);
    colon++;
    values[idx++] = colon;
    // Serial.print("Found value ");
    // Serial.println(colon);
    kvPair = strtok(0, "&");
  }

  Serial.print("Found ");
  Serial.print(numParams);
  Serial.println(" Parameters");

  for (int i = 0; i < numParams; i++) {
    Serial.print(keys[i]);
    Serial.print(" = ");
    Serial.println(values[i]);
  }

  return numParams;
}

void SimpleWebServer::handleRequest() {

  if (!started) {
    started = true;
    server.begin();
  }
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("Client Found! Reading request");
  String method = client.readStringUntil(' ');
  Serial.println(method);
  String route = client.readStringUntil(' ');

  int numParams = 0;
  char** keys;
  char** values;
  char* params;
  if (method == "POST") {
    String line;
    do {
      line = client.readStringUntil('\r');
      line.trim();
      // Serial.print("Skipping till post body: [");
      // Serial.print(line);
      // Serial.println("]");
    }
    while(line.length() > 0);

    String paramString = client.readStringUntil('\0');
    paramString.trim();
    // Serial.print("POST Params: [");
    // Serial.print(paramString);
    // Serial.println("]");
    if (paramString.length() > 0) {
      params = new char[paramString.length() + 1];
      params[paramString.length()] = 0;
      strcpy(params, paramString.c_str());
      // Serial.println("Parsing params");
      numParams = parseParams(params, keys, values);
      // Serial.println("Done params");
    }
  }
  if (method == "GET") {
    int idx;
    if ((idx = route.indexOf('?')) != -1) {
      String paramString = route.substring(idx + 1);
      paramString.trim();
      if (paramString.length() > 0) {
        params = new char[paramString.length() + 1];
        params[paramString.length()] = 0;
        strcpy(params, paramString.c_str());
        numParams = parseParams(params, keys, values);
      }
    }
  }

  Serial.println("Looking for route: ");
  Serial.println(method);
  Serial.println(route);

  for (int i = 0; i < numRoutes; i++) {
    if (method == routes[i]->method && route == routes[i]->route) {
      Serial.println("Found route!");
      (* routes[i]->response)(client, numParams, keys, values);
    }
  }

  client.flush();

  if (numParams > 0) {
    delete [] params;
    delete [] keys;
    delete [] values;
  }
}
