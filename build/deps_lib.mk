message_writer.o: src/ares/message_writer.cpp src/ares/message_writer.h \
  src/ares/buffer.h src/ares/error.h src/ares/exception.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h src/ares/bytes.h \
  src/ares/packet_writer.h src/ares/buffer_formatter.h src/ares/sink.h \
  src/ares/bin_util.h
sockfd_poller.o: src/ares/sockfd_poller.cpp src/ares/sockfd_poller.h \
  src/ares/types.h src/ares/utility.h src/ares/error.h \
  src/ares/exception.h src/ares/net_tk.h src/ares/network_common.h \
  src/ares/config.h
message_session.o: src/ares/message_session.cpp \
  src/ares/message_session.h src/ares/buffer.h src/ares/error.h \
  src/ares/exception.h src/ares/shared_ptr.h src/ares/guard.h \
  src/ares/mutex.h src/ares/platform.h src/ares/config.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h src/ares/message_reader.h \
  src/ares/buffer_formatter.h src/ares/sink.h src/ares/session.h \
  src/ares/date.h src/ares/sync_queue.h src/ares/fixed_allocator.h \
  src/ares/command.h src/ares/server_interface.h src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/utility.h src/ares/socket.h src/ares/trace.h
error.o: src/ares/error.cpp src/ares/error.h src/ares/exception.h
server.o: src/ares/server.cpp src/ares/server.h src/ares/component.h \
  src/ares/date.h src/ares/types.h src/ares/thread.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/server_interface.h src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/utility.h src/ares/session.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/rwlock.h src/ares/sink.h \
  src/ares/command_queue.h src/ares/shared_queue.h \
  src/ares/auto_inc_dec.h src/ares/condition.h src/ares/null_mutex.h \
  src/ares/sync_queue.h src/ares/fixed_allocator.h src/ares/dispatcher.h \
  src/ares/buffer.h src/ares/listener.h src/ares/service.h \
  src/ares/socket_acceptor.h src/ares/sockfd_poller.h src/ares/log.h \
  src/ares/socket.h src/ares/processor.h src/ares/receiver.h \
  src/ares/string_util.h src/ares/trace.h
line_reader.o: src/ares/line_reader.cpp src/ares/line_reader.h \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/buffer.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/rwlock.h
data_reader.o: src/ares/data_reader.cpp src/ares/data_reader.h \
  src/ares/basic_reader.h src/ares/buffer_formatter.h src/ares/error.h \
  src/ares/exception.h src/ares/utility.h src/ares/types.h \
  src/ares/bytes.h src/ares/buffer.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/rwlock.h src/ares/bin_util.h
guard.o: src/ares/guard.cpp src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/error.h \
  src/ares/exception.h src/ares/types.h src/ares/utility.h \
  src/ares/rwlock.h
buffer.o: src/ares/buffer.cpp src/ares/buffer.h src/ares/error.h \
  src/ares/exception.h src/ares/shared_ptr.h src/ares/guard.h \
  src/ares/mutex.h src/ares/platform.h src/ares/config.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h
shared_ptr.o: src/ares/shared_ptr.cpp src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/error.h src/ares/exception.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h
cmdline_arg_parser.o: src/ares/cmdline_arg_parser.cpp \
  src/ares/cmdline_arg_parser.h src/ares/error.h src/ares/exception.h \
  src/ares/string_util.h src/ares/types.h src/ares/utility.h
mutex.o: src/ares/mutex.cpp src/ares/mutex.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/types.h src/ares/utility.h
component.o: src/ares/component.cpp src/ares/component.h src/ares/date.h \
  src/ares/types.h src/ares/thread.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/log.h src/ares/socket.h src/ares/buffer.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/rwlock.h src/ares/string_util.h
random.o: src/ares/random.cpp src/ares/random.h
condition.o: src/ares/condition.cpp src/ares/condition.h \
  src/ares/platform.h src/ares/config.h src/ares/error.h \
  src/ares/exception.h src/ares/types.h src/ares/utility.h \
  src/ares/mutex.h
sequence.o: src/ares/sequence.cpp src/ares/sequence.h \
  src/ares/null_mutex.h src/ares/utility.h
socket.o: src/ares/socket.cpp src/ares/socket.h src/ares/buffer.h \
  src/ares/error.h src/ares/exception.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/utility.h src/ares/rwlock.h src/ares/date.h \
  src/ares/net_tk.h src/ares/string_util.h
bytes.o: src/ares/bytes.cpp src/ares/bytes.h src/ares/types.h
dispatcher.o: src/ares/dispatcher.cpp src/ares/dispatcher.h \
  src/ares/buffer.h src/ares/error.h src/ares/exception.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h src/ares/component.h \
  src/ares/date.h src/ares/thread.h src/ares/session.h src/ares/sink.h \
  src/ares/shared_queue.h src/ares/auto_inc_dec.h src/ares/condition.h \
  src/ares/null_mutex.h src/ares/sync_queue.h src/ares/fixed_allocator.h \
  src/ares/command.h src/ares/log.h src/ares/socket.h \
  src/ares/server_interface.h src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/utility.h src/ares/string_util.h src/ares/trace.h
session.o: src/ares/session.cpp src/ares/session.h src/ares/date.h \
  src/ares/types.h src/ares/shared_ptr.h src/ares/guard.h \
  src/ares/mutex.h src/ares/platform.h src/ares/config.h src/ares/error.h \
  src/ares/exception.h src/ares/utility.h src/ares/rwlock.h \
  src/ares/sink.h src/ares/buffer.h src/ares/sequence.h \
  src/ares/null_mutex.h src/ares/server_interface.h \
  src/ares/job/scheduler.h src/ares/job/common.h src/ares/job/interval.h \
  src/ares/date.h src/ares/utility.h src/ares/socket.h \
  src/ares/string_util.h src/ares/trace.h
exception.o: src/ares/exception.cpp src/ares/exception.h \
  src/ares/string_tokenizer.h src/ares/error.h src/ares/utility.h \
  src/ares/string_util.h src/ares/types.h
receiver.o: src/ares/receiver.cpp src/ares/receiver.h \
  src/ares/command_queue.h src/ares/shared_queue.h \
  src/ares/auto_inc_dec.h src/ares/condition.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/types.h src/ares/utility.h src/ares/null_mutex.h \
  src/ares/sync_queue.h src/ares/fixed_allocator.h src/ares/guard.h \
  src/ares/mutex.h src/ares/rwlock.h src/ares/component.h src/ares/date.h \
  src/ares/thread.h src/ares/server_interface.h src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/utility.h src/ares/session.h src/ares/shared_ptr.h \
  src/ares/sink.h src/ares/sockfd_poller.h src/ares/command.h \
  src/ares/buffer.h src/ares/log.h src/ares/socket.h \
  src/ares/string_util.h src/ares/trace.h
string_tokenizer.o: src/ares/string_tokenizer.cpp \
  src/ares/string_tokenizer.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h
processor.o: src/ares/processor.cpp src/ares/command.h src/ares/buffer.h \
  src/ares/error.h src/ares/exception.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/utility.h src/ares/rwlock.h \
  src/ares/session.h src/ares/date.h src/ares/sink.h src/ares/log.h \
  src/ares/socket.h src/ares/processor.h src/ares/command_queue.h \
  src/ares/shared_queue.h src/ares/auto_inc_dec.h src/ares/condition.h \
  src/ares/null_mutex.h src/ares/sync_queue.h src/ares/fixed_allocator.h \
  src/ares/component.h src/ares/thread.h src/ares/string_util.h \
  src/ares/trace.h
command.o: src/ares/command.cpp src/ares/command.h src/ares/buffer.h \
  src/ares/error.h src/ares/exception.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/utility.h src/ares/rwlock.h \
  src/ares/session.h src/ares/date.h src/ares/sink.h \
  src/ares/server_interface.h src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/utility.h src/ares/socket.h src/ares/trace.h
service.o: src/ares/service.cpp src/ares/service.h \
  src/ares/listener_strategy.h
socket_acceptor.o: src/ares/socket_acceptor.cpp \
  src/ares/socket_acceptor.h src/ares/sockfd_poller.h src/ares/types.h \
  src/ares/utility.h src/ares/error.h src/ares/exception.h \
  src/ares/net_tk.h src/ares/socket.h src/ares/buffer.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/rwlock.h src/ares/date.h \
  src/ares/trace.h
math_util.o: src/ares/math_util.cpp src/ares/math_util.h
data_writer.o: src/ares/data_writer.cpp src/ares/data_writer.h \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/bytes.h src/ares/types.h src/ares/buffer.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/rwlock.h \
  src/ares/bin_util.h
string_util.o: src/ares/string_util.cpp src/ares/string_util.h \
  src/ares/types.h src/ares/error.h src/ares/exception.h
trace.o: src/ares/trace.cpp src/ares/trace.h src/ares/config.h \
  src/ares/date.h src/ares/types.h src/ares/error.h src/ares/exception.h \
  src/ares/file_util.h src/ares/string_tokenizer.h src/ares/utility.h \
  src/ares/string_util.h src/ares/thread.h src/ares/platform.h
pid_lock.o: src/ares/pid_lock.cpp src/ares/pid_lock.h src/ares/error.h \
  src/ares/exception.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/utility.h src/ares/string_util.h
log.o: src/ares/log.cpp src/ares/log.h src/ares/socket.h \
  src/ares/buffer.h src/ares/error.h src/ares/exception.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h src/ares/date.h \
  src/ares/shared_queue.h src/ares/auto_inc_dec.h src/ares/condition.h \
  src/ares/null_mutex.h src/ares/sync_queue.h src/ares/fixed_allocator.h \
  src/ares/string_util.h src/ares/thread.h src/ares/trace.h
date_util.o: src/ares/date_util.cpp src/ares/date_util.h \
  src/ares/bin_util.h src/ares/types.h src/ares/bytes.h src/ares/date.h
fixed_allocator.o: src/ares/fixed_allocator.cpp \
  src/ares/fixed_allocator.h src/ares/utility.h
file_util.o: src/ares/file_util.cpp src/ares/file_util.h src/ares/error.h \
  src/ares/exception.h src/ares/string_tokenizer.h src/ares/utility.h \
  src/ares/string_util.h src/ares/types.h
bin_util.o: src/ares/bin_util.cpp src/ares/bin_util.h src/ares/types.h
date.o: src/ares/date.cpp src/ares/date.h src/ares/types.h \
  src/ares/error.h src/ares/exception.h src/ares/string_util.h
packet_writer.o: src/ares/packet_writer.cpp src/ares/packet_writer.h \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/buffer.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/rwlock.h src/ares/data_writer.h \
  src/ares/bytes.h src/ares/bin_util.h
thread.o: src/ares/thread.cpp src/ares/thread.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/types.h src/ares/utility.h
net_tk.o: src/ares/net_tk.cpp src/ares/net_tk.h src/ares/types.h \
  src/ares/error.h src/ares/exception.h src/ares/network_common.h \
  src/ares/config.h src/ares/utility.h
http.o: src/ares/http/http.cpp src/ares/http/http.h \
  src/ares/string_util.h src/ares/types.h
error.o: src/ares/http/error.cpp src/ares/http/error.h src/ares/error.h \
  src/ares/exception.h src/ares/http/http.h
request_parser.o: src/ares/http/request_parser.cpp \
  src/ares/http/request_parser.h src/ares/http/http.h \
  src/ares/http/header_table.h src/ares/string_util.h src/ares/types.h \
  src/ares/string_tokenizer.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/http/error.h src/ares/error.h \
  src/ares/http/request.h
header_table.o: src/ares/http/header_table.cpp \
  src/ares/http/header_table.h src/ares/http/http.h \
  src/ares/string_util.h src/ares/types.h
request.o: src/ares/http/request.cpp src/ares/http/request.h \
  src/ares/http/http.h src/ares/http/header_table.h \
  src/ares/string_util.h src/ares/types.h
shared_queue.o: src/ares/shared_queue.cpp src/ares/shared_queue.h \
  src/ares/auto_inc_dec.h src/ares/condition.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/types.h src/ares/utility.h src/ares/null_mutex.h \
  src/ares/sync_queue.h src/ares/fixed_allocator.h src/ares/guard.h \
  src/ares/mutex.h src/ares/rwlock.h
message_reader.o: src/ares/message_reader.cpp src/ares/message_reader.h \
  src/ares/buffer.h src/ares/error.h src/ares/exception.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/types.h \
  src/ares/utility.h src/ares/rwlock.h src/ares/buffer_formatter.h \
  src/ares/sink.h src/ares/data_reader.h src/ares/basic_reader.h \
  src/ares/bytes.h
platform.o: src/ares/platform.cpp src/ares/platform.h src/ares/config.h \
  src/ares/error.h src/ares/exception.h src/ares/types.h \
  src/ares/utility.h src/ares/string_tokenizer.h src/ares/string_util.h
command_queue.o: src/ares/command_queue.cpp src/ares/command_queue.h \
  src/ares/shared_queue.h src/ares/auto_inc_dec.h src/ares/condition.h \
  src/ares/platform.h src/ares/config.h src/ares/error.h \
  src/ares/exception.h src/ares/types.h src/ares/utility.h \
  src/ares/null_mutex.h src/ares/sync_queue.h src/ares/fixed_allocator.h \
  src/ares/guard.h src/ares/mutex.h src/ares/rwlock.h
utility.o: src/ares/utility.cpp src/ares/utility.h
packet_reader.o: src/ares/packet_reader.cpp src/ares/packet_reader.h \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/buffer.h src/ares/shared_ptr.h \
  src/ares/guard.h src/ares/mutex.h src/ares/platform.h src/ares/config.h \
  src/ares/types.h src/ares/rwlock.h src/ares/bin_util.h
rwlock.o: src/ares/rwlock.cpp src/ares/rwlock.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/types.h src/ares/utility.h
basic_reader.o: src/ares/basic_reader.cpp src/ares/basic_reader.h \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/types.h src/ares/buffer.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/rwlock.h
listener.o: src/ares/listener.cpp src/ares/listener.h \
  src/ares/command_queue.h src/ares/shared_queue.h \
  src/ares/auto_inc_dec.h src/ares/condition.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/types.h src/ares/utility.h src/ares/null_mutex.h \
  src/ares/sync_queue.h src/ares/fixed_allocator.h src/ares/guard.h \
  src/ares/mutex.h src/ares/rwlock.h src/ares/component.h src/ares/date.h \
  src/ares/thread.h src/ares/service.h src/ares/socket_acceptor.h \
  src/ares/sockfd_poller.h src/ares/listener_strategy.h src/ares/log.h \
  src/ares/socket.h src/ares/buffer.h src/ares/shared_ptr.h \
  src/ares/server_interface.h src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/utility.h src/ares/session.h src/ares/sink.h \
  src/ares/string_util.h src/ares/trace.h
basic_writer.o: src/ares/basic_writer.cpp src/ares/basic_writer.h \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h src/ares/types.h src/ares/buffer.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/rwlock.h
sockfd_selector.o: src/ares/sockfd_selector.cpp \
  src/ares/sockfd_selector.h src/ares/types.h src/ares/utility.h \
  src/ares/error.h src/ares/exception.h src/ares/net_tk.h \
  src/ares/network_common.h src/ares/config.h
error.o: src/ares/job/error.cpp src/ares/job/error.h src/ares/error.h \
  src/ares/exception.h
job.o: src/ares/job/job.cpp src/ares/job/job.h src/ares/job/common.h \
  src/ares/job/interval.h src/ares/date.h src/ares/types.h \
  src/ares/thread.h src/ares/platform.h src/ares/config.h \
  src/ares/error.h src/ares/exception.h src/ares/utility.h
scheduler.o: src/ares/job/scheduler.cpp src/ares/job/scheduler.h \
  src/ares/job/common.h src/ares/job/interval.h src/ares/date.h \
  src/ares/types.h src/ares/utility.h src/ares/job/error.h \
  src/ares/error.h src/ares/exception.h src/ares/job/job.h \
  src/ares/thread.h src/ares/platform.h src/ares/config.h \
  src/ares/error.h src/ares/utility.h src/ares/job/job_queue.h \
  src/ares/guard.h src/ares/mutex.h src/ares/rwlock.h src/ares/mutex.h \
  src/ares/platform.h src/ares/sequence.h src/ares/null_mutex.h \
  src/ares/shared_queue.h src/ares/auto_inc_dec.h src/ares/condition.h \
  src/ares/sync_queue.h src/ares/fixed_allocator.h src/ares/guard.h
interval.o: src/ares/job/interval.cpp src/ares/job/interval.h \
  src/ares/date.h src/ares/types.h
job_queue.o: src/ares/job/job_queue.cpp src/ares/job/job_queue.h \
  src/ares/job/job.h src/ares/job/common.h src/ares/job/interval.h \
  src/ares/date.h src/ares/types.h src/ares/thread.h src/ares/platform.h \
  src/ares/config.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h
sink.o: src/ares/sink.cpp src/ares/sink.h src/ares/types.h \
  src/ares/buffer.h src/ares/error.h src/ares/exception.h \
  src/ares/shared_ptr.h src/ares/guard.h src/ares/mutex.h \
  src/ares/platform.h src/ares/config.h src/ares/utility.h \
  src/ares/rwlock.h
buffer_formatter.o: src/ares/buffer_formatter.cpp \
  src/ares/buffer_formatter.h src/ares/error.h src/ares/exception.h \
  src/ares/utility.h
