use tokio::net::{TcpListener, TcpStream};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use std::error::Error;

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let listener = TcpListener::bind("127.0.0.1:8080").await?;
    println!("Server running on 127.0.0.1:8080");

    loop {
        let (mut socket, _) = listener.accept().await?;

        tokio::spawn(async move {
            loop {
                // Read the length of the incoming message
                let mut len_buf = [0; 4];
                if let Err(_) = socket.read_exact(&mut len_buf).await {
                    return;
                }
                let len = u32::from_le_bytes(len_buf);

                // Read the body of the message
                let mut buf = vec![0; len as usize];
                if let Err(_) = socket.read_exact(&mut buf).await {
                    return;
                }

                // Echo the message back to the client
                if let Err(_) = socket.write_all(&len_buf).await {
                    return;
                }
                if let Err(_) = socket.write_all(&buf).await {
                    return;
                }
            }
        });
    }
}
