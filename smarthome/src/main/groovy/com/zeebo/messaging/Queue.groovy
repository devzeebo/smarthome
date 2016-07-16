package com.zeebo.messaging

import java.util.concurrent.ArrayBlockingQueue
import java.util.concurrent.TimeUnit
import java.util.concurrent.locks.Lock

class MessageQueue {

	interface MessageCallback {
		void callback(Message q)
	}

	def name

	ArrayBlockingQueue<Message> messages = new ArrayBlockingQueue<>(1000)

	def queueMessage(Message message) {
		while(!messages.offer(message, 1, TimeUnit.SECONDS));
	}

	def handleMessage(MessageCallback c) {
		def message
		while((message = messages.poll(1, TimeUnit.SECONDS)) == null);
		println "Handling message ${message}"
		c.callback(message)
	}
}
