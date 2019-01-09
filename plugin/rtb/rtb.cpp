#include <plugin.h>

#include <google/protobuf/util/json_util.h>
#include <RTB.pb.h>
#include <BAIDU.pb.h> 
#include <TENCENT.pb.h> 
#include <WIFI.pb.h>

namespace
{

#define pb2json(type, from, to) \
	do {\
		type pbMessage; \
		if(pbMessage.ParseFromString(from)) \
		{ \
			serializeToJson(pbMessage, to); \
			return true; \
		} \
	}while(0)

class RTBHandler : public HttpHandler
{
public:
	bool handleHttpMessage(const Tuple4 & tuple4, bool direction, HttpMessage & message) override
	{
		if(message.isRequest)
		{
			if(message.url == "/djt")
			{
				pb2json(RTB::Request, message.body, message.body);
			}
			else if(message.url == "/baidu_djt")
			{
				pb2json(BAIDU::BidRequest, message.body, message.body);
			}
			else if(message.url == "/tencent_djt")
			{
				pb2json(TENCENT::Request, message.body, message.body);
			}
			else if(message.url == "/wifi_djt")
			{
				pb2json(WIFI::Request, message.body, message.body);
			}
		}
		else
		{
			pb2json(RTB::Response, message.body, message.body);
			pb2json(BAIDU::BidResponse, message.body, message.body);
			pb2json(TENCENT::Response, message.body, message.body);
			pb2json(WIFI::Response, message.body, message.body);
		}

		return false;
	}

	~RTBHandler() override
	{

	}

private:
	bool serializeToJson(const google::protobuf::Message & message, std::string & output)
	{
		google::protobuf::util::JsonPrintOptions option;
		option.add_whitespace = true;
		std::string str;
		auto result = google::protobuf::util::MessageToJsonString(message, &str, option);
		if(result.ok())
		{
			output = std::move(str);
			return true;
		}
		return false;
	}

};

}

HttpHandlerPtr createHttpHandler()
{
	return std::make_shared<RTBHandler>();
}


