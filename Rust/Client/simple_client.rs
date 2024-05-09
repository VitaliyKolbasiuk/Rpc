use tokio::net::TcpStream;
use tokio::io::{AsyncReadExt, AsyncWriteExt};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Connect to the server
    let mut stream = TcpStream::connect("127.0.0.1:8080").await?;

    // Message to send
    let message_body = "Hello from Rust client!";
    let message_length = message_body.len() as u32;

    // Send the message length followed by the message body
    stream.write_all(&message_length.to_le_bytes()).await?;
    stream.write_all(message_body.as_bytes()).await?;

    // Read the response length
    let mut response_length_buf = [0; 4];
    stream.read_exact(&mut response_length_buf).await?;
    let response_length = u32::from_le_bytes(response_length_buf);

    // Read the response body
    let mut response_body = vec![0; response_length as usize];
    stream.read_exact(&mut response_body).await?;

    // Convert response body to string
    let response_str = String::from_utf8(response_body)?;

    println!("Reply from server: {}", response_str);

    Ok(())
}
