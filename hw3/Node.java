import java.nio.file.InvalidPathException;
import java.io.File;
import java.util.*;

public class Node
{
    protected List<Node> children = null;
    private Node parent = null;
    public long size = 0;
    public String filename;
    private File file;

    public Node(Node parent, String filepath)
    {
        filename = filepath;
        this.parent = parent;
        file = new File(filepath);
        // if (! file.isDirectory())
        //     throw new InvalidPathException(filename, "It is not a directory.");
        this.children = new ArrayList<>();
        createChildren();
        calculateSize();
    }

    public void createChildren()
    {
        if (file.isFile())
            return;

        File[] pathnames = file.listFiles();
        for (File pathname : pathnames)
            {
                Node child = new Node(this, pathname.getAbsolutePath());
                child.createChildren();
                children.add(child);
            }
    }

    public long calculateSize()
    {
        if (file.isFile())
            {
                this.size = this.file.length();
                return this.size;
            }
        else
            {
                for (Node child : children)
                    this.size += child.calculateSize();
                return this.size;
            }
    }

    public boolean representsFile()
    {
        if (! file.isDirectory())
            return true;
        else
            return false;
    }

    public void print_tree()
    {
        if (this == null)
            return;
        System.out.println(filename + ": " + Long.toString(this.size));
        if (children == null)
            return;
        for (Node child : children)
            {
                child.print_tree();
            }
    }
}
