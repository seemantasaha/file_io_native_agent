public class Test {

	public static int f(int z) {
	        System.out.println("Hello from f()!");
		return z*z;
	}

	public static void main(String a[]) throws InterruptedException {
	        System.out.println("Hello!");
		int x = f(7);
	        System.out.println("Goodbye!");
	}

}
