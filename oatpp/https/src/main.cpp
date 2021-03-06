//
//  main.cpp
//  benchmark-http
//
//  Created by Leonid on 9/7/18.
//  Copyright © 2018 lganzzzo. All rights reserved.
//

#include "Logger.hpp"

#include "oatpp/web/protocol/http/outgoing/BufferBody.hpp"

#include "oatpp/web/protocol/http/incoming/Request.hpp"
#include "oatpp/web/protocol/http/outgoing/Response.hpp"

#include "oatpp-libressl/server/ConnectionProvider.hpp"
#include "oatpp-libressl/Config.hpp"
#include "oatpp-libressl/Callbacks.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/server/Server.hpp"
#include "oatpp/core/Types.hpp"

#include <iostream>
#include <csignal>

/* typedef for convenience */
typedef oatpp::web::protocol::http::Status Status;
typedef oatpp::web::protocol::http::incoming::Request Request;
typedef oatpp::web::protocol::http::outgoing::Response Response;

/**
 * Handler to handle requests for URL to which it subscribed via Router
 */
class Handler : public oatpp::web::url::mapping::Subscriber<std::shared_ptr<Request>, std::shared_ptr<Response>> {
public:
  
  /**
   * method to handle Synchronous requests
   */
  std::shared_ptr<Response> processUrl(const std::shared_ptr<Request>& request) override {
    auto body = oatpp::web::protocol::http::outgoing::BufferBody::createShared("Hello World!!!");
    return Response::createShared(Status::CODE_200, body);
  }
  
  /**
   * method to handle Asynchronous requests (NOT USED HERE)
   */
  Action processUrlAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                         AsyncCallback callback,
                         const std::shared_ptr<Request>& request) override {
    throw std::runtime_error("this method is not used in the test");
  }
  
};

/**
 *  Run server method
 */
void run() {
  
  /* ignore SIGPIPE */
  std::signal(SIGPIPE, SIG_IGN);
  
  /* set default callbacks for libressl */
  oatpp::libressl::Callbacks::setDefaultCallbacks();
  
  /* create libressl config */
  // if you get:
  // runtime_error failed call to tls_config_set_key_file() - double check
  // key and cert file paths.
  
  auto key = "../../cert/test_key.pem"; // full path to .pem file
  auto cert = "../../cert/test_cert.crt"; // full path to .crt file
  auto config = oatpp::libressl::Config::createDefaultServerConfig(key, cert);
  
  /* init server components */
  auto connectionProvider = oatpp::libressl::server::ConnectionProvider::createShared(config, 8443);
  auto router = oatpp::web::server::HttpRouter::createShared();
  auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
  
  /* Handler subscribed to the root URL */
  router->addSubscriber("GET", "/", std::make_shared<Handler>());
  
  /* create server and run server */
  oatpp::network::server::Server server(connectionProvider, connectionHandler);
  
  OATPP_LOGD("app", "multithreaded server running on port %s", connectionProvider->getProperty("port").toString()->c_str());
  
  server.run();
  
}

/**
 * main()
 * init Environment and run server
 */
int main(int argc, const char * argv[]) {
  
  oatpp::base::Environment::init();
  oatpp::base::Environment::setLogger(new Logger());
  run();
  oatpp::base::Environment::setLogger(nullptr);
  oatpp::base::Environment::destroy();
  
  return 0;
  
}
