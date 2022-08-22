#pragma once
#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace last
{
    namespace net
    {
        template <typename T>
        class server_interface;

        template <typename T>
        class connection : public std::enable_shared_from_this<connection<T>>
        {
        public:
            enum class owner
            {
                server,
                client
            };

        protected:
            asio::ip::tcp::socket m_socket;
            asio::io_context &m_asioContext;
            tsqueue<message<T>> m_qMessagesOut;
            tsqueue<owned_message<T>> &m_qMessagesIn;
            message<T> m_msgTemporaryIn;
            owner m_OwnerType = owner::server;
            uint32_t id = 0;

            uint64_t m_nHandShakeOut = 0;
            uint64_t m_nHandShakeIn = 0;
            uint64_t m_nHandShakeCheck = 0;

        public:
            connection(owner parent, asio::io_context &asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>> &qIn)
                : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
            {
                m_OwnerType = parent;

                if(m_OwnerType == owner::server)
                {
                    m_nHandShakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
                    m_nHandShakeCheck = scramble(m_nHandShakeOut);

                }
                else
                {
                    m_nHandShakeOut = 0;
                    m_nHandShakeCheck = 0;
                }
            }
            virtual ~connection() {}

            uint32_t GetID() const
            {
                return id;
            }

            void ConnectToClient(last::net::server_interface<T>* server ,uint32_t uid = 0)
            {
                if (m_OwnerType == owner::server)
                    if (m_socket.is_open())
                    {
                        id = uid;
                        WriteValidation();
                        ReadValidation(server);
                    }
            }
            bool ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
            {
                if(m_OwnerType == owner::client)
                asio::async_connect(m_socket, endpoints,
                [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                {
                    if(!ec)
                        ReadValidation();
                });
            }
            bool Disconnect()
            {
                if (IsConnected())
                    asio::post(m_asioContext, [this]()
                               { m_socket.close(); });
            }
            bool IsConnected() const
            {
                return m_socket.is_open();
            }

            void Send(const message<T> &msg)
            {
                asio::post(m_asioContext,
                           [this, msg]()
                           {
                               bool bWritingMessage = !m_qMessagesOut.empty();
                               m_qMessagesOut.push_back(msg);
                               if (!bWritingMessage)
                                   WriteHeader();
                           });
            }

        private:
            void ReadHeader()
            {
                asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
                                 [this](std::error_code ec, std::size_t length)
                                 {
                                     if (!ec)
                                     {
                                         if (m_msgTemporaryIn.header.size > 0)
                                         {
                                             m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                             ReadBody();
                                         }
                                         else
                                         {
                                             AddToIncomingMessageQueue();
                                         }
                                     }
                                     else
                                     {
                                         std::cout << "[" << id << "] Read Header Fail\n";
                                         m_socket.close();
                                     }
                                 });
            }

            void ReadBody()
            {
                asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                                 [this](std::error_code ec, std::size_t length)
                                 {
                                     if (!ec)
                                     {
                                         AddToIncomingMessageQueue();
                                     }
                                     else
                                     {
                                         std::cout << "[" << id << "] Read Body Fail\n";
                                         m_socket.close();
                                     }
                                 });
            }

            void WriteHeader()
            {
                asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
                                  [this](std::error_code ec, std::size_t length)
                                  {
                                      if (!ec)
                                      {
                                          if (m_qMessagesOut.front().body.size() > 0)
                                          {
                                              WriteBody();
                                          }
                                          else
                                          {
                                              m_qMessagesOut.pop_front();
                                                //Forgot the "!"...
                                              if (!m_qMessagesOut.empty())
                                                  WriteHeader();
                                          }
                                      }
                                      else
                                      {
                                          std::cout << "[" << id << "] Write Header Fail.\n";
                                          m_socket.close();
                                      }
                                  });
            }

            void WriteBody()
            {
                asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                                  [this](std::error_code ec, std::size_t length)
                                  {
                                      if (!ec)
                                      {
                                          m_qMessagesOut.pop_front();

                                          if (!m_qMessagesOut.empty())
                                              WriteHeader();
                                      }
                                      else
                                      {
                                          std::cout << "[" << id << "] Write Body Fail.\n";
                                          m_socket.close();
                                      }
                                  });
            }

            void AddToIncomingMessageQueue()
            {
                if (m_OwnerType == owner::server)
                    m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
                else
                    m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});

                ReadHeader();
            }

            uint64_t scramble(uint64_t nInput)
            {
                uint64_t out = nInput ^ 0x10DECADEDEADBEEF;
                //Other nice options:
                //C0FFEEBABE, DEFACE, DECAFC0FFEE
                //B01DFACE, CA11AB1E, CA55E11E, 5CA1AB1E

                out = (out ^ 0xC0FEEBABE) >> 4 | (out ^ 0xDECAFC0FFE) << 4;
                return out ^ 0xDEADFACEC0DECAFE;               
            }

            void WriteValidation()
            {
                asio::async_write(m_socket, asio::buffer(&m_nHandShakeOut, sizeof(uint64_t)),
                [this](asio::error_code ec, std::size_t length)
                {
                    if(!ec)
                    {
                        if(m_OwnerType == owner::client)
                            ReadHeader();
                    }
                    else
                    {
                        m_socket.close();
                    }
                });
            }

            void ReadValidation(last::net::server_interface<T>* server = nullptr)
            {
                asio::async_read(m_socket, asio::buffer(&m_nHandShakeIn, sizeof(uint64_t)),
                [this, server](asio::error_code ec, size_t length)
                {
                    if(!ec)
                    {
                        if(m_OwnerType == owner::server)
                        {
                            if(m_nHandShakeIn == m_nHandShakeCheck)
                            {
                                std::cout << "Client Validated\n";
                                server->OnClientValidated(this->shared_from_this());

                                ReadHeader();
                            }
                            else
                            {
                                std::cout << "Client Disconnected (Fail Validation)\n";
                                m_socket.close();
                            }
                        }
                        else
                        {
                            m_nHandShakeOut = scramble(m_nHandShakeIn);
                            WriteValidation();
                        }
                    }
                    else
                    {
                        std::cout << "Client Disconnected (ReadValidation)\n";
                        m_socket.close();
                    }
                });
            }
        };
    }
}