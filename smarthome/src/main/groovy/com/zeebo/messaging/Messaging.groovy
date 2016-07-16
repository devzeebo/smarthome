package com.zeebo.messaging

class Messaging {

	private def queues = [:]

	MessageQueue getAt(String queueName) {
		if (!queues[queueName]) {
			queues[queueName] = new MessageQueue(name: queueName)
		}
		return queues[queueName]
	}
}
