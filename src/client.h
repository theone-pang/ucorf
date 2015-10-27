#pragma once

#include "preheader.h"
#include "message.h"
#include "transport.h"
#include "option.h"
#include "server_finder.h"

namespace ucorf
{
    class IServiceStub;
    class ITransportClient;
    class IDispatcher;
    class Client
    {
    public:
        typedef boost::function<ITransportClient*()> TransportFactory;

        Client();
        ~Client();

        Client& SetOption(boost::shared_ptr<Option> opt);

        Client& SetDispatcher(std::unique_ptr<IDispatcher> && dispatcher);

        Client& SetHeaderFactory(HeaderFactory const& head_factory);

        Client& SetServerFinder(std::unique_ptr<ServerFinder> && srv_finder);

        Client& SetTransportFactory(TransportFactory const& factory);
        
        Client& SetUrl(std::string const& url);

        boost_ec Call(std::string const& service_name,
                std::string const& method_name,
                IMessage *request, IMessage *response);

    private:
        void OnConnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id);
        void OnDisconnected(boost::shared_ptr<ITransportClient> tp, SessId sess_id, boost_ec const& ec);
        size_t OnReceiveData(boost::shared_ptr<ITransportClient> tp, SessId sess_id, const char* data, size_t bytes);
        void OnResponse(boost::shared_ptr<ITransportClient> tp, IHeaderPtr header, const char* data, size_t bytes);

    private:
        struct ResponseData
        {
            boost_ec ec;
            IHeaderPtr header;
            std::vector<char> data;

            ResponseData() = default;
            ResponseData(boost_ec const& e) : ec(e) {}
        };

        typedef std::map<std::string, ITransportClient*> StubMap;
        typedef co_chan<ResponseData> RspChan;
        typedef std::unordered_map<ITransportClient*, std::unordered_map<std::size_t, RspChan>> ChannelMap;
        StubMap stubs_;
        std::string url_;
        ChannelMap channels_;
        HeaderFactory head_factory_;
        TransportFactory tp_factory_;
        std::unique_ptr<ServerFinder> srv_finder_;
        std::unique_ptr<IDispatcher> dispatcher_;
        std::atomic<std::size_t> msg_id_{0};
        boost::shared_ptr<Option> opt_;
    };

} //namespace ucorf