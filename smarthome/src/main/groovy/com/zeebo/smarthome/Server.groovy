package com.zeebo.smarthome

import com.zeebo.messaging.Message
import com.zeebo.messaging.Messaging

class Server {


	public static void main(String[] args) {

		def messaging = new Messaging()

		def ss = new ServerSocket(7788)

		ss.accept(true) {
			println "Accepted connection from ${it.inetAddress.hostAddress}"
			def queue = messaging[it.inetAddress.hostAddress]
			it.withStreams { inputStream, outputStream ->
				while(true) {
					queue.handleMessage {
						println "Got message: ${it.content}"
						outputStream.write "${it.content}\0".bytes
						outputStream.flush()
					}
				}
			}
		}

		Scanner scan = new Scanner(System.in)
		while(true) {
			def line = scan.nextLine().split(' ')

			messaging[line[0]].queueMessage(new Message(content: "${line[1]}${line[2]}"))
		}
	}
}
