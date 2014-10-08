/*
===========================================================================
Daemon BSD Source Code
Copyright (c) 2013-2014, Daemon Developers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Daemon developers nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL DAEMON DEVELOPERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
===========================================================================
*/

#ifndef COMMON_IPC_H_
#define COMMON_IPC_H_

namespace IPC {

// Operating system handle type
#ifdef _WIN32
// HANDLE is defined as void* in windows.h
// Note that the NaCl code uses -1 instead of 0 to represent invalid handles
typedef void* OSHandleType;
const OSHandleType INVALID_HANDLE = reinterpret_cast<void*>(-1);
#else
typedef int OSHandleType;
const OSHandleType INVALID_HANDLE = -1;
#endif

// IPC descriptor which can be sent over a socket. You should treat this as an
// opaque type and not access any of the fields directly.
#ifdef __native_client__
typedef int Desc;
#else
struct Desc {
	OSHandleType handle;
	int type;
	union {
		uint64_t size;
		int32_t flags;
	};
};
#endif
void CloseDesc(const Desc& desc);

// Simple file handle wrapper, does *not* close handle on destruction
enum FileOpenMode {
	MODE_READ,
	MODE_WRITE,
	MODE_RW,
	MODE_WRITE_APPEND,
	MODE_RW_APPEND
};
class FileHandle {
public:
	FileHandle()
		: handle(INVALID_HANDLE) {}
	FileHandle(OSHandleType handle, FileOpenMode mode)
		: handle(handle), mode(mode) {}
	explicit operator bool() const
	{
		return handle != INVALID_HANDLE;
	}

	Desc GetDesc() const;
	static FileHandle FromDesc(const Desc& desc);

	OSHandleType GetHandle() const
	{
		return handle;
	}

private:
	OSHandleType handle;
	FileOpenMode mode;
};

// Message-based socket through which data and handles can be passed.
class Reader;
class Writer;
class Socket {
public:
	Socket()
		: handle(INVALID_HANDLE) {}
	Socket(Socket&& other) NOEXCEPT
		: handle(other.handle)
	{
		other.handle = INVALID_HANDLE;
	}
	Socket& operator=(Socket&& other) NOEXCEPT
	{
		std::swap(handle, other.handle);
		return *this;
	}
	~Socket()
	{
		Close();
	}
	explicit operator bool() const
	{
		return handle != INVALID_HANDLE;
	}

	void Close();

	OSHandleType GetHandle() const
	{
		return handle;
	}
	OSHandleType ReleaseHandle()
	{
		OSHandleType out = handle;
		handle = INVALID_HANDLE;
		return out;
	}

	Desc GetDesc() const;
	static Socket FromDesc(const Desc& desc);
	static Socket FromHandle(OSHandleType handle);

	void SendMsg(const Writer& writer) const;
	Reader RecvMsg() const;

	static std::pair<Socket, Socket> CreatePair();

private:
	OSHandleType handle;
};

// Shared memory area, can be sent over a socket
class SharedMemory {
public:
	SharedMemory()
		: handle(INVALID_HANDLE) {}
	SharedMemory(SharedMemory&& other) NOEXCEPT
		: handle(other.handle), base(other.base), size(other.size)
	{
		other.handle = INVALID_HANDLE;
	}
	SharedMemory& operator=(SharedMemory&& other) NOEXCEPT
	{
		std::swap(handle, other.handle);
		std::swap(base, other.base);
		std::swap(size, other.size);
		return *this;
	}
	~SharedMemory()
	{
		Close();
	}
	explicit operator bool() const
	{
		return handle != INVALID_HANDLE;
	}

	void Close();

	Desc GetDesc() const;
	static SharedMemory FromDesc(const Desc& desc);

	static SharedMemory Create(size_t size);

	void* GetBase() const
	{
		return base;
	}
	size_t GetSize() const
	{
		return size;
	}

private:
	void Map();
	OSHandleType handle;
	void* base;
	size_t size;
};

// Base type for serialization traits.
template<typename T, typename = void> struct SerializeTraits {};

// Class to generate messages
class Writer {
public:
	void WriteData(const void* p, size_t len)
	{
		data.insert(data.end(), static_cast<const char*>(p), static_cast<const char*>(p) + len);
	}
	void WriteSize(size_t size)
	{
		if (size > std::numeric_limits<uint32_t>::max())
			Com_Error(ERR_DROP, "IPC: Size out of range in message");
		Write<uint32_t>(size);
	}
	template<typename T, typename Arg> void Write(Arg&& value)
	{
		SerializeTraits<T>::Write(*this, std::forward<Arg>(value));
	}
	void WriteHandle(const Desc& h)
	{
		handles.push_back(h);
	}

	const std::vector<char>& GetData() const
	{
		return data;
	}
	const std::vector<Desc>& GetHandles() const
	{
		return handles;
	}

private:
	std::vector<char> data;
	std::vector<Desc> handles;
};

// Class to read messages
class Reader {
public:
	Reader()
		: pos(0), handles_pos(0) {}
	Reader(Reader&& other) NOEXCEPT
		: data(std::move(other.data)), handles(std::move(other.handles)), pos(other.pos), handles_pos(other.handles_pos) {}
	Reader& operator=(Reader&& other) NOEXCEPT
	{
		std::swap(data, other.data);
		std::swap(handles, other.handles);
		std::swap(pos, other.pos);
		std::swap(handles_pos, other.handles_pos);
		return *this;
	}
	~Reader()
	{
		// Close any handles that weren't read
		for (size_t i = handles_pos; i < handles.size(); i++)
			CloseDesc(handles[i]);
	}

	void ReadData(void* p, size_t len)
	{
		if (pos + len <= data.size()) {
			memcpy(p, data.data() + pos, len);
			pos += len;
		} else
			Com_Error(ERR_DROP, "IPC: Unexpected end of message");
	}
	template<typename T> size_t ReadSize()
	{
		// Check for multiplication overflows when reading a size
		uint32_t size;
		ReadData(&size, sizeof(uint32_t));
		if (size > std::numeric_limits<uint32_t>::max() / sizeof(T))
			Com_Error(ERR_DROP, "IPC: Size out of range in message");
		return size;
	}
	const void* ReadInline(size_t len)
	{
		if (pos + len <= data.size()) {
			const void* out = data.data() + pos;
			pos += len;
			return out;
		} else
			Com_Error(ERR_DROP, "IPC: Unexpected end of message");
	}
	template<typename T> decltype(SerializeTraits<T>::Read(std::declval<Reader&>())) Read()
	{
		return SerializeTraits<T>::Read(*this);
	}
	Desc ReadHandle()
	{
		if (handles_pos <= handles.size())
			return handles[handles_pos++];
		else
			Com_Error(ERR_DROP, "IPC: Unexpected end of message");
	}

	std::vector<char>& GetData()
	{
		return data;
	}
	std::vector<Desc>& GetHandles()
	{
		return handles;
	}

private:
	std::vector<char> data;
	std::vector<Desc> handles;
	size_t pos;
	size_t handles_pos;
};

// Simple implementation for POD types
template<typename T>
struct SerializeTraits<T, typename std::enable_if<std::is_pod<T>::value && !std::is_array<T>::value>::type> {
	static void Write(Writer& stream, const T& value)
	{
		stream.WriteData(std::addressof(value), sizeof(value));
	}
	static T Read(Reader& stream)
	{
		T value;
		stream.ReadData(std::addressof(value), sizeof(value));
		return value;
	}
};

// std::array for non-POD types (POD types are already handled by the base case)
template<typename T, size_t N>
struct SerializeTraits<std::array<T, N>, typename std::enable_if<!std::is_pod<T>::value>::type> {
	static void Write(Writer& stream, const std::array<T, N>& value)
	{
		for (const T& x: value)
			stream.Write<T>(x);
	}
	static std::array<T, N> Read(Reader& stream)
	{
		std::array<T, N> value;
		for (T& x: value)
			x = stream.Read<T>();
		return value;
	}
};

// std::vector, with a specialization for POD types
template<typename T>
struct SerializeTraits<std::vector<T>, typename std::enable_if<std::is_pod<T>::value>::type> {
	static void Write(Writer& stream, const std::vector<T>& value)
	{
		stream.WriteSize(value.size());
		stream.WriteData(value.data(), value.size() * sizeof(T));
	}
	static std::vector<T> Read(Reader& stream)
	{
		std::vector<T> value;
		value.resize(stream.ReadSize<T>());
		stream.ReadData(value.data(), value.size() * sizeof(T));
		return value;
	}
};
template<typename T>
struct SerializeTraits<std::vector<T>, typename std::enable_if<!std::is_pod<T>::value>::type> {
	static void Write(Writer& stream, const std::vector<T>& value)
	{
		stream.WriteSize(value.size());
		for (const T& x: value)
			stream.Write<T>(x);
	}
	static std::vector<T> Read(Reader& stream)
	{
		std::vector<T> value;
		value.resize(stream.ReadSize<T>());
		for (T& x: value)
			x = stream.Read<T>();
		return value;
	}
};

// std::pair
template<typename T, typename U>
struct SerializeTraits<std::pair<T, U>> {
	static void Write(Writer& stream, const std::pair<T, U>& value)
	{
		stream.Write<T>(value.first);
		stream.Write<U>(value.second);
	}
	static std::pair<T, U> Read(Reader& stream)
	{
		std::pair<T, U> value;
		value.first = stream.Read<T>();
		value.second = stream.Read<U>();
		return value;
	}
};

// Util::optional
template<typename T>
struct SerializeTraits<Util::optional<T>> {
	static void Write(Writer& stream, const Util::optional<T>& value)
	{
		stream.Write<bool>(bool(value));
		if (value)
			stream.Write<T>(*value);
	}
	static Util::optional<T> Read(Reader& stream)
	{
		if (stream.Read<bool>())
			return stream.Read<T>();
		else
			return Util::nullopt;
	}
};

// std::string
template<> struct SerializeTraits<std::string> {
	static void Write(Writer& stream, Str::StringRef value)
	{
		stream.WriteSize(value.size());
		stream.WriteData(value.data(), value.size());
	}
	static std::string Read(Reader& stream)
	{
		size_t size = stream.ReadSize<char>();
		const char* p = static_cast<const char*>(stream.ReadInline(size));
		return std::string(p, p + size);
	}
};

// std::map and std::unordered_map
template<typename T, typename U>
struct SerializeTraits<std::map<T, U>> {
	static void Write(Writer& stream, const std::map<T, U>& value)
	{
		stream.WriteSize(value.size());
		for (const std::pair<T, U>& x: value)
			stream.Write<std::pair<T, U>>(x);
	}
	static std::map<T, U> Read(Reader& stream)
	{
		std::map<T, U> value;
		size_t size = stream.ReadSize<std::pair<T, U>>();
		for (size_t i = 0; i != size; i++)
			value.insert(stream.Read<std::pair<T, U>>());
		return value;
	}
};
template<typename T, typename U>
struct SerializeTraits<std::unordered_map<T, U>> {
	static void Write(Writer& stream, const std::unordered_map<T, U>& value)
	{
		stream.WriteSize(value.size());
		for (const std::pair<T, U>& x: value)
			stream.Write<std::pair<T, U>>(x);
	}
	static std::unordered_map<T, U> Read(Reader& stream)
	{
		std::unordered_map<T, U> value;
		size_t size = stream.ReadSize<std::pair<T, U>>();
		for (size_t i = 0; i != size; i++)
			value.insert(stream.Read<std::pair<T, U>>());
		return value;
	}
};

// std::set and std::unordered_set
template<typename T>
struct SerializeTraits<std::set<T>> {
	static void Write(Writer& stream, const std::set<T>& value)
	{
		stream.WriteSize(value.size());
		for (const T& x: value)
			stream.Write<T>(x);
	}
	static std::set<T> Read(Reader& stream)
	{
		std::set<T> value;
		size_t size = stream.ReadSize<T>();
		for (size_t i = 0; i != size; i++)
			value.insert(stream.Read<T>());
		return value;
	}
};
template<typename T>
struct SerializeTraits<std::unordered_set<T>> {
	static void Write(Writer& stream, const std::unordered_set<T>& value)
	{
		stream.WriteSize(value.size());
		for (const T& x: value)
			stream.Write<T>(x);
	}
	static std::unordered_set<T> Read(Reader& stream)
	{
		std::unordered_set<T> value;
		size_t size = stream.ReadSize<T>();
		for (size_t i = 0; i != size; i++)
			value.insert(stream.Read<T>());
		return value;
	}
};

// Socket, file handle and shared memory
template<> struct SerializeTraits<Socket> {
	static void Write(Writer& stream, const Socket& value)
	{
		stream.WriteHandle(value.GetDesc());
	}
	static Socket Read(Reader& stream)
	{
		return Socket::FromDesc(stream.ReadHandle());
	}
};
template<> struct SerializeTraits<FileHandle> {
	static void Write(Writer& stream, const FileHandle& value)
	{
		stream.WriteHandle(value.GetDesc());
	}
	static FileHandle Read(Reader& stream)
	{
		return FileHandle::FromDesc(stream.ReadHandle());
	}
};
template<> struct SerializeTraits<SharedMemory> {
	static void Write(Writer& stream, const SharedMemory& value)
	{
		stream.WriteHandle(value.GetDesc());
	}
	static SharedMemory Read(Reader& stream)
	{
		return SharedMemory::FromDesc(stream.ReadHandle());
	}
};

// Message ID to indicate an RPC return
const uint32_t ID_RETURN = 0xffffffff;

// Combine a major and minor ID into a single number
template<uint16_t Major, uint16_t Minor> struct Id {
	enum {
		value = (Major << 16) + Minor
	};
};

// An IPC channel wraps a socket and provides additional support for sending
// synchronous typed messages over it.
class Channel {
public:
	Channel()
		: counter(0) {}
	Channel(Socket socket)
		: socket(std::move(socket)), counter(0) {}
	Channel(Channel&& other)
		: socket(std::move(other.socket)) {}
	Channel& operator=(Channel&& other)
	{
		std::swap(socket, other.socket);
		return *this;
	}
	explicit operator bool() const
	{
		return bool(socket);
	}

	// Wrappers around socket functions
	void SendMsg(const Writer& writer) const
	{
		socket.SendMsg(writer);
	}
	Reader RecvMsg() const
	{
		return socket.RecvMsg();
	}

	// Generate a unique message key to match messages with replies
	uint32_t GenMsgKey()
	{
		return counter++;
	}

	// Wait for a synchronous message reply, returns the message ID and contents
	std::pair<uint32_t, Reader> RecvReplyMsg(uint32_t key)
	{
		// If we have already recieved a reply for this key, just return that
		auto it = replies.find(key);
		if (it != replies.end()) {
			Reader reader = std::move(it->second);
			replies.erase(it);
			return {ID_RETURN, std::move(reader)};
		}

		// Otherwise handle incoming messages until we find the reply we want
		while (true) {
			Reader reader = RecvMsg();

			// Is this a reply?
			uint32_t id = reader.Read<uint32_t>();
			if (id != ID_RETURN)
				return {id, std::move(reader)};

			// Is this the reply we are expecting?
			uint32_t msg_key = reader.Read<uint32_t>();
			if (msg_key == key)
				return {ID_RETURN, std::move(reader)};

			// Save the reply for later
			replies.insert(std::make_pair(msg_key, std::move(reader)));
		}
	}

private:
	Socket socket;
	uint32_t counter;
	std::unordered_map<uint32_t, Reader> replies;
};

// Asynchronous message which does not wait for a reply
template<typename Id, typename... T> struct Message {
	enum {
		id = Id::value
	};
	typedef std::tuple<T...> Inputs;
};

// Reply class which should only be used for the second parameter of SyncMessage
template<typename... T> struct Reply {
	typedef std::tuple<T...> Outputs;
};

// Synchronous message which waits for a reply. The reply can contain data.
template<typename Msg, typename Reply = Reply<>> struct SyncMessage {
	enum {
		id = Msg::id
	};
	typedef typename Msg::Inputs Inputs;
	typedef typename Reply::Outputs Outputs;
};

namespace detail {

// Serialize a list of types into a Writer (ignores extra trailing arguments)
template<typename... Args> void SerializeArgs(Util::TypeList<>, Writer&, Args&&...) {}
template<typename Type0, typename... Types, typename Arg0, typename... Args> void SerializeArgs(Util::TypeList<Type0, Types...>, Writer& writer, Arg0&& arg0, Args&&... args)
{
	writer.Write<Type0>(std::forward<Arg0>(arg0));
	SerializeArgs(Util::TypeList<Types...>(), writer, std::forward<Args>(args)...);
}
template<typename Types, typename Tuple, size_t... Seq> void SerializeTuple(Types, Writer& writer, Tuple&& tuple, Util::seq<Seq...>)
{
	SerializeArgs(Types(), writer, std::get<Seq>(std::forward<Tuple>(tuple))...);
}
template<typename Types, typename Tuple> void SerializeTuple(Types, Writer& writer, Tuple&& tuple)
{
	SerializeTuple(Types(), writer, std::forward<Tuple>(tuple), Util::gen_seq<std::tuple_size<typename std::decay<Tuple>::type>::value>());
}

// Read a list of types into a tuple starting at the given index
template<size_t Index, typename Tuple> void FillTuple(Util::TypeList<>, Tuple&, Reader&) {}
template<size_t Index, typename Type0, typename... Types, typename Tuple> void FillTuple(Util::TypeList<Type0, Types...>, Tuple& tuple, Reader& stream)
{
	std::get<Index>(tuple) = stream.Read<Type0>();
	FillTuple<Index + 1>(Util::TypeList<Types...>(), tuple, stream);
}

// Map a tuple to get the actual types returned by SerializeTraits::Read instead of the declared types
template<typename T> struct MapTupleHelper {
	typedef decltype(SerializeTraits<T>::Read(std::declval<Reader&>())) type;
};
template<typename T> struct MapTuple {};
template<typename... T> struct MapTuple<std::tuple<T...>> {
	typedef std::tuple<typename MapTupleHelper<T>::type...> type;
};

// Implementations of SendMsg for Message and SyncMessage
template<typename Func, typename Id, typename... MsgArgs, typename... Args> void SendMsg(Channel& channel, Func&&, Message<Id, MsgArgs...>, Args&&... args)
{
	typedef Message<Id, MsgArgs...> Message;
	static_assert(sizeof...(Args) == std::tuple_size<typename Message::Inputs>::value, "Incorrect number of arguments for IPC::SendMsg");

	Writer writer;
	writer.Write<uint32_t>(Message::id);
	SerializeArgs(Util::TypeListFromTuple<typename Message::Inputs>(), writer, std::forward<Args>(args)...);
	channel.SendMsg(writer);
}
template<typename Func, typename Msg, typename Reply, typename... Args> void SendMsg(Channel& channel, Func&& messageHandler, SyncMessage<Msg, Reply>, Args&&... args)
{
	typedef SyncMessage<Msg, Reply> Message;
	static_assert(sizeof...(Args) == std::tuple_size<typename Message::Inputs>::value + std::tuple_size<typename Message::Outputs>::value, "Incorrect number of arguments for IPC::SendMsg");

	Writer writer;
	writer.Write<uint32_t>(Message::id);
	uint32_t key = channel.GenMsgKey();
	writer.Write<uint32_t>(key);
	SerializeArgs(Util::TypeListFromTuple<typename Message::Inputs>(), writer, std::forward<Args>(args)...);
	channel.SendMsg(writer);

	while (true) {
		Reader reader;
		uint32_t id;
		std::tie(id, reader) = channel.RecvReplyMsg(key);
		if (id == ID_RETURN) {
			auto out = std::forward_as_tuple(std::forward<Args>(args)...);
			FillTuple<std::tuple_size<typename Message::Inputs>::value>(Util::TypeListFromTuple<typename Message::Outputs>(), out, reader);
			return;
		}
		messageHandler(id, std::move(reader));
	}
}

// Implementations of HandleMsg for Message and SyncMessage
template<typename Func, typename Id, typename... MsgArgs> void HandleMsg(Channel&, Message<Id, MsgArgs...>, IPC::Reader reader, Func&& func)
{
	typedef Message<Id, MsgArgs...> Message;

	typename MapTuple<typename Message::Inputs>::type inputs;
	FillTuple<0>(Util::TypeListFromTuple<typename Message::Inputs>(), inputs, reader);
	Util::apply(std::forward<Func>(func), std::move(inputs));
}
template<typename Func, typename Msg, typename Reply> void HandleMsg(Channel& channel, SyncMessage<Msg, Reply>, IPC::Reader reader, Func&& func)
{
	typedef SyncMessage<Msg, Reply> Message;

	uint32_t key = reader.Read<uint32_t>();
	typename MapTuple<typename Message::Inputs>::type inputs;
	typename Message::Outputs outputs;
	FillTuple<0>(Util::TypeListFromTuple<typename Message::Inputs>(), inputs, reader);
	Util::apply(std::forward<Func>(func), std::tuple_cat(Util::ref_tuple(std::move(inputs)), Util::ref_tuple(outputs)));

	Writer writer;
	writer.Write<uint32_t>(ID_RETURN);
	writer.Write<uint32_t>(key);
	SerializeTuple(Util::TypeListFromTuple<typename Message::Outputs>(), writer, std::move(outputs));
	channel.SendMsg(writer);
}

} // namespace detail

// Send a message to the given channel. If the message is synchronous then messageHandler is invoked for all
// message that are recieved until ID_RETURN is recieved. Values returned by a synchronous message are
// returned through reference parameters.
template<typename Msg, typename Func, typename... Args> void SendMsg(Channel& channel, Func&& messageHandler, Args&&... args)
{
	detail::SendMsg(channel, messageHandler, Msg(), std::forward<Args>(args)...);
}

// Handle an incoming message using a callback function (which can just be a lambda). If the message is
// synchronous then outputs values are written to using reference parameters.
template<typename Msg, typename Func> void HandleMsg(Channel& channel, Reader reader, Func&& func)
{
	detail::HandleMsg(channel, Msg(), std::move(reader), std::forward<Func>(func));
}

} // namespace IPC

#endif // COMMON_IPC_H_
