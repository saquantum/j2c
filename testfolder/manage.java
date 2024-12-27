public class manage{
    public static void main(String[] args){
        student s = new student("lcl", false, 27, "001");
        while(s.getAge()>0){
            System.out.println(s);
            s.setAge(s.getAge()-1);
        }
    }
    public static <E extends student> boolean isStudent(E e){
        return e instanceof student;
    }
}
