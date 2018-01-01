import java.io.PrintWriter;
import java.io.IOException;

public class TestPrintWriter {

	public static void main(String[] args) {
		try {
			PrintWriter writer = new PrintWriter("output.txt", "UTF-8");
			for(int j=0; j<5; j++) {
				String s = new String(new char[j]).replace("\0", "~");
				System.out.println("Going to write: " + s);
				writer.println(s);
			}
			writer.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
	}

}
