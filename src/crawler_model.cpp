#include "crawler_model.h"
#include "message_types.h"

namespace CrawlerImpl
{

void CrawlerModel::saveUniqueUrls(const TagParser& tagParser, const Url& hostUrl, const Url& containingUrl)
{
	Url currentUrl;
	WarningMessage httpsWarning{ "Current HTTP library does not support HTTPS protocol" };

	assert(tagParser.countTag("a") == tagParser.size());

	for (const Tag& tag : tagParser)
	{
		try
		{

			currentUrl.parse(tag.attribute("href"));

			if (currentUrl.fileType() != Url::FileType::ExecutableWebFile ||
				currentUrl.isAnchor())
			{
				continue;
			}

			if (currentUrl.isAbsoluteAddress() && !currentUrl.compareHost(hostUrl))
			{
				if (currentUrl.protocol() != "https://")
				{
					sendMessage(&httpsWarning);
					continue;
				}

				if (m_externalCrawledUrlQueue.find(currentUrl) != std::end(m_externalCrawledUrlQueue))
				{
					m_externalUrlQueue.emplace(currentUrl.host());

					sendMessage(&QueueItersAndRefsInvalidatedMessage{ ExternalUrlQueue });
				}

				continue;
			}

			if (currentUrl.isAbsoluteAddress() && currentUrl.compareHost(hostUrl))
			{
				if (currentUrl.protocol() != "https://")
				{
					sendMessage(&httpsWarning);
					continue;
				}

				if (m_internalCrawledUrlQueue.find(currentUrl) != std::end(m_internalCrawledUrlQueue))
				{
					m_internalUrlQueue.emplace(currentUrl.relativePath());

					sendMessage(&QueueItersAndRefsInvalidatedMessage{ InternalUrlQueue });
				}

				continue;
			}

			//
			// WARNING:
			// There is a possibility that there will be 2 identical addresses: / and /index.php often equivalent
			//

			Url url = currentUrl.mergeRelativePaths(currentUrl, containingUrl);

			if (m_internalCrawledUrlQueue.find(currentUrl) == std::end(m_internalCrawledUrlQueue))
			{
				m_internalUrlQueue.emplace(url.relativePath());

				sendMessage(&QueueItersAndRefsInvalidatedMessage{ InternalUrlQueue });
			}
		}
		catch (UrlParseErrorException const& parsingUrlError)
		{
			sendMessage(&WarningMessage{ parsingUrlError.what() });
		}
	}

	sendMessage(&QueueSizeMessage{ InternalUrlQueue, size(InternalUrlQueue) });
}

void CrawlerModel::storeUrl(const Url& url, int queueType)
{
	queue(queueType)->emplace(url);
}

CrawlerModel::SmartModelElementPtr CrawlerModel::anyUrl(int queueType)
{
	SmartModelElementPtr pointer{ new SmartModelElement(std::addressof(*queue(queueType)->begin()), this, queueType) };

	addReceiver(pointer.get());

	return pointer;
}

std::size_t CrawlerModel::size(int queueType) const noexcept
{
	return queue(queueType)->size();
}

CrawlerModel::SmartModelElementPtr CrawlerModel::moveUrl(const Url& urlKey, int fromQueueType, int toQueueType)
{
	Queue::iterator iter = queue(fromQueueType)->find(urlKey);

	assert(iter != std::end(*queue(fromQueueType)));

	std::pair<Queue::iterator, bool> insertionResult = 
		queue(toQueueType)->emplace(std::move(*iter));

	queue(fromQueueType)->erase(iter);

	sendMessage(&QueueItersAndRefsInvalidatedMessage{ fromQueueType });
	sendMessage(&QueueItersAndRefsInvalidatedMessage{ toQueueType });

	SmartModelElementPtr pointer{ new SmartModelElement(std::addressof(*insertionResult.first), this, toQueueType) };
	addReceiver(pointer.get());

	return pointer;
}

CrawlerModel::Queue* CrawlerModel::queue(int queueType) noexcept
{
	const Queue* queuePointer = 
		const_cast<CrawlerModel const * const>(this)->queue(queueType);
	
	return const_cast<Queue*>(queuePointer);
}

const CrawlerModel::Queue* CrawlerModel::queue(int queueType) const noexcept
{
	assert(queueType == InternalUrlQueue ||
		queueType == InternalCrawledUrlQueue ||
		queueType == ExternalUrlQueue ||
		queueType == ExternalCrawledUrlQueue
	);

	if (queueType == InternalUrlQueue)
	{
		return &m_internalUrlQueue;
	}

	if (queueType == InternalCrawledUrlQueue)
	{
		return &m_internalCrawledUrlQueue;
	}

	if (queueType == ExternalUrlQueue)
	{
		return &m_externalUrlQueue;
	}

	return &m_externalUrlQueue;
}

CrawlerModel::SmartModelElement::SmartModelElement(const Url* url, CrawlerModel* model, int queueType)
	: m_url(url)
	, m_model(model)
	, m_pointerInvalidated(false)
	, m_queueType(queueType)
{
}

CrawlerModel::SmartModelElement::~SmartModelElement()
{
	m_model->deleteReceiver(this);
}

void CrawlerModel::SmartModelElement::receiveMessage(const IMessage* message)
{
	if (message->type() == IMessage::MessageType::QueueItersAndRefsInvalidated)
	{
		const QueueItersAndRefsInvalidatedMessage* invalidatedItersAndRefsMessage =
				static_cast<const QueueItersAndRefsInvalidatedMessage*>(message);

		if (invalidatedItersAndRefsMessage->queueType() == m_queueType)
		{
			m_pointerInvalidated = true;
		}
	}
}

const HtmlParser::Url* CrawlerModel::SmartModelElement::value() const noexcept
{
	assert(!m_pointerInvalidated);

	return m_url;
}

}