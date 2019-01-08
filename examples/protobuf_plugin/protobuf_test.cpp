#include <plugin.h>
#include <test.pb.h>

namespace
{

class ProtobufTestHandler : public HttpHandler
{
public:
	bool handleHttpMessage(const Tuple4 & tuple4, bool direction, HttpMessage & message) override
	{
		test::Company company;
		if(!company.ParseFromString(message.body))
		{
			return false;
		}

		message.body = company.DebugString();
		return true;
	}

	~ProtobufTestHandler() override
	{

	}
};

}

HttpHandlerPtr createHttpHandler()
{
	return std::make_shared<ProtobufTestHandler>();
}


