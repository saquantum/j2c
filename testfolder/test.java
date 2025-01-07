public class test<E> implements Comparable<E>{
    public static void main(String[] args){
        System.out.printf("abc\n");
    }
    
    @Override
    public int compareTo(E e){
        return hashCode();
    }
}
