import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.BufferedOutputStream;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.io.ByteArrayOutputStream;
import java.io.FilterOutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;
import java.nio.file.StandardOpenOption;

public class WriteToFileExample {

	//private static final String FILEPATH = "testFile.txt";

	public static void main(String[] args) throws IOException {

		String str1 = "abc";
		String str2 = "asdasfasfasfa";

		List<String> list = new ArrayList<String>();
		list.add(str1);
		list.add(str2);

		// Writing examples
		System.out.println("Starting a test for all file writing operation begins");
		usePrintWriter("hello", "test/testFile0.txt");
		useFileWriter(str1, "test/testFile1.txt");
		useBufferedFileWriter(list, "test/testFile2.txt");
		useFileOutPutStream(str2, "test/testFile3.txt");
		useBufferedFileOutPutStream(list, "test/testFile4.txt");
		useBufferedOutPutStream(list, "test/testFile5.txt");
		useByteArrayOutputStream("Test for ByteArrayOutputStream", "test/testfile6.txt");
		useFilterOutputStream("test/testfile7.txt");
		//Files.write(Paths.get("testfile8.txt"), str2.getBytes(), StandardOpenOption.CREATE);
		System.out.println("Starting a test for all file writing operation ends");

		// Redaing Examples
		useFileInputStream("test/test_input1.txt");
		//List<String> lines = Files.readAllLines(Paths.get("test/test_input2.txt"));

	}


	public static void useFileInputStream(String filePath) {
	  FileInputStream in = null;

	  try {
		  in = new FileInputStream(filePath);
		 
		  int c;
		  while ((c = in.read()) != -1) {
		    //System.out.println(c);
		  }
		  if (in != null) {
		    in.close();
		  }
	  } catch (IOException e) {
			e.printStackTrace();
	  } finally {
	  }
	}

// Writing examples
	public static void usePrintWriter(String content, String filepath) {
		try {
			PrintWriter writer = new PrintWriter(filepath, "UTF-8");
			writer.println(content);
			writer.close();
		} catch(IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Write a small string to a File - Use a FileWriter
	 */
	public static void useFileWriter(String content, String filePath) {
		Writer writer = null;

		try {

			writer = new FileWriter(filePath);
			writer.write(content);

		} catch (IOException e) {

			System.err.println("Error writing the file : ");
			e.printStackTrace();

		} finally {

			if (writer != null) {
				try {
					writer.close();
				} catch (IOException e) {

					System.err.println("Error closing the file : ");
					e.printStackTrace();
				}
			}

		}
	}

	/**
	 * Write a big list of Strings to a file - Use a BufferedWriter
	 */
	public static void useBufferedFileWriter(List<String> content,
			String filePath) {

		File file = new File(filePath);
		Writer fileWriter = null;
		BufferedWriter bufferedWriter = null;

		try {

			fileWriter = new FileWriter(file);
			bufferedWriter = new BufferedWriter(fileWriter);

			// Write the lines one by one
			for (String line : content) {
				line += System.getProperty("line.separator");
				bufferedWriter.write(line);

				// alternatively add bufferedWriter.newLine() to change line
			}

		} catch (IOException e) {
			System.err.println("Error writing the file : ");
			e.printStackTrace();
		} finally {

			if (bufferedWriter != null && fileWriter != null) {
				try {
					bufferedWriter.close();
					fileWriter.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

	}

	/**
	 * Write raw data to a small file - use FileOutPutStream
	 */
	public static void useFileOutPutStream(String content, String filePath) {

		OutputStream outputStream = null;

		try {

			outputStream = new FileOutputStream(new File(filePath));
			//outputStream.write(content.getBytes(), 0, content.length());
			outputStream.write(64);

		} catch (FileNotFoundException e) {
			System.err.println("Error Opening the file : ");
			e.printStackTrace();
		} catch (IOException e) {
			System.err.println("Error writing  the file : ");
			e.printStackTrace();
		} finally {

			if (outputStream != null) {
				try {
					outputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

		}

	}

	/**
	 * Write character data to a big file - use BufferedWriter
	 */
	public static void useBufferedFileOutPutStream(List<String> content, String filePath) {
		Writer writer = null;

		try {

			// Using OutputStreamWriter you don't have to convert the String to byte[]
			writer = new BufferedWriter(new OutputStreamWriter(
					new FileOutputStream(filePath), "utf-8"));

			for (String line : content) {
				line += System.getProperty("line.separator");
				writer.write(line);
			}

		} catch (IOException e) {

		} finally {

			if (writer != null) {
				try {
					writer.close();
				} catch (Exception e) {

				}
			}
		}
	}
        /**
	 * Write raw data to a big file - use BufferedOutputStream
	 */
	public static void useBufferedOutPutStream(List<String> content,String filePath) {
		BufferedOutputStream bout = null;
		try {

			
			bout = new BufferedOutputStream( new FileOutputStream(filePath) );

			for (String line : content) {
				line += System.getProperty("line.separator");
				bout.write(line.getBytes());
			}

		} catch (IOException e) {

		} finally {

			if (bout != null) {
				try {
					bout.close();
				} catch (Exception e) {

				}
			}
		}

	}


	public static void useByteArrayOutputStream(String content, String filepath) {
		try {
			ByteArrayOutputStream f = new ByteArrayOutputStream();
	    
			byte buf[] = content.getBytes();
			f.write(buf);
			byte b[] = f.toByteArray();
			OutputStream f2 = new FileOutputStream(filepath);
			f.writeTo(f2);
			f2.close();
		} catch (IOException e) {
		}
	}

	public static void useFilterOutputStream(String filepath) {
		OutputStream os = null;
		FilterOutputStream fos = null;
		try {
			byte[] buffer = {65, 66, 67, 68, 69};
			os = new FileOutputStream(filepath);
			fos = new FilterOutputStream(os);
			fos.write(buffer);
			os.close();
			fos.close();
		} catch (IOException e) {
		}
	}

}
