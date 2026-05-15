#include "runtime/drast_runtime.hpp"

struct Message;

struct Message_Text {
    std::string body;
};
struct Message_Count {
    int amount;
};
struct Message {
    enum class Tag {
        Text,
        Count,
    };
    Tag tag;
    std::variant<Message_Text, Message_Count> data;
    static Message Text(std::string body) {
        Message _out;
        _out.tag = Tag::Text;
        _out.data = Message_Text{std::move(body)};
        return _out;
    }
    static Message Count(int amount) {
        Message _out;
        _out.tag = Tag::Count;
        _out.data = Message_Count{std::move(amount)};
        return _out;
    }
};

std::string describe(Message& message);

std::string describe(Message& message) {
    {
        const auto& _match = message;
        if (_match == Message::Text(body)) {
            return body;
        }
        else if (_match == Message::Count(amount)) {
            return "count";
        }
    }
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    Text message = Message::Text("hello");
    std::string description = describe(message);
    return description.size();
}

