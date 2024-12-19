/*
a test file.
*/
public class student{
    private String name;
    private boolean gender;
    private int age;
    private String id;
    
    // empty constructor.
    public student(){
    }
    
    public student(String name, boolean gender, int age, String id){
        this.name = name;
        this.gender = gender;
        this.age = age;
        this.id = id;
    }
    
    public String getName(){
        return name;
    }
    public void setName(){
        this.name = name;
    }
    
    public boolean getGender(){
        return gender;
    }
    public void setGender(){
        this.gender = gender;
    }
    
    public int getAge(){
        return age;
    }
    public void setAge(){
        this.age = age;
    }
    
    public String getId(){
        return id;
    }
    public void setId(){
        this.id = id;
    }
}
