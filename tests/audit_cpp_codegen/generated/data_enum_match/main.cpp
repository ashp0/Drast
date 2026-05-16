#include <string>
#include <utility>
#include <variant>

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

std::string describe(const Message& message);

std::string describe(const Message& message) {
    {
        const auto& _match = message;
        switch (_match.tag) {
        case Message::Tag::Text: {
            const auto& _payload = std::get<Message_Text>(_match.data);
            const auto& body = _payload.body;
            return body;
            break;
        }
        case Message::Tag::Count: {
            const auto& _payload = std::get<Message_Count>(_match.data);
            const auto& amount = _payload.amount;
            return "count";
            break;
        }
        }
        __builtin_unreachable();
    }
}

int main() {
    Message message = Message::Text("hello");
    std::string description = describe(std::move(message));
    return description.size();
}

