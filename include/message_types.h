#pragma once

#include "stdlibs.h"
#include "url.h"

namespace CrawlerImpl
{

class IMessage
{
public:
	enum class MessageType
	{
		CurrentRequestedUrl,
		CurrentQueueSize,
		QueueItersAndRefsInvalidated,
		HttpResponseCode,
		WarningType
	};

	virtual MessageType type() const noexcept = 0;
};

class WarningMessage : public IMessage
{
public:
	WarningMessage(const std::string& warning)
		: m_warning(warning)
	{
	}

	virtual MessageType type() const noexcept override
	{
		return MessageType::WarningType;
	}

	const std::string& warning() const noexcept
	{
		return m_warning;
	}

private:
	std::string m_warning;
};

class HttpResponseCodeMessage : public IMessage
{
public:
	HttpResponseCodeMessage(int code)
		: m_codeResponse(code)
	{
	}

	virtual MessageType type() const noexcept override
	{
		return MessageType::HttpResponseCode;
	}

	int code() const noexcept
	{
		return m_codeResponse;
	}

private:
	int m_codeResponse;
};

class QueueItersAndRefsInvalidatedMessage : public IMessage
{
public:
	QueueItersAndRefsInvalidatedMessage(int queueType)
		: m_queueType(queueType)
	{
	}

	virtual MessageType type() const noexcept override
	{
		return MessageType::QueueItersAndRefsInvalidated;
	}

	int queueType() const noexcept
	{
		return m_queueType;
	}

private:
	int m_queueType;
};

class QueueSizeMessage : public IMessage
{
public:
	QueueSizeMessage(int queueType, std::size_t size)
		: m_queueType(queueType)
		, m_size(size)
	{
	}

	virtual MessageType type() const noexcept override
	{
		return MessageType::CurrentQueueSize;
	}

	int queueType() const noexcept
	{
		return m_queueType;
	}

	std::size_t size() const noexcept
	{
		return m_size;
	}

private:
	int m_queueType;
	std::size_t m_size;
};

//
// TODO: need to omit url parsing when creating this object
//
class CurrentRequestedUrlMessage : public IMessage
{
public:
	CurrentRequestedUrlMessage(const std::string& url)
		: m_url(url)
	{
	}

	const std::string& url() const noexcept
	{
		return m_url;
	}

	virtual MessageType type() const noexcept override
	{
		return MessageType::CurrentRequestedUrl;
	}

private:
	std::string m_url;
};

}