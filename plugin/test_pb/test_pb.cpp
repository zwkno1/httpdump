#include <plugin.h>
#include <test.pb.h>
#include <google/protobuf/util/json_util.h>
#include <boost/dll.hpp>

namespace
{

class ProtobufTestHandler : public HttpHandler
{
public:
    bool handleHttpRequest(const Tuple4 & tuple4, HttpRequest & message) override
	{
		test::Company company;
        if(!company.ParseFromString(message.body()))
		{
			return false;
		}
		
		std::string text;
		google::protobuf::util::JsonPrintOptions option;
		option.add_whitespace = true;
		google::protobuf::util::MessageToJsonString(company, &text, option);
        message.body() = std::move(text);
		return true;
    }

    bool handleHttpResponse(const Tuple4 & tuple4, HttpResponse & message) override
    {
        return false;
    }
};

}

HttpHandlerPtr createHttpHandler()
{
	return std::make_shared<ProtobufTestHandler>();
}

