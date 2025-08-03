fn main() -> Result<(), Box<dyn std::error::Error>>
{
    let zips_dir = std::path::Path::new("data/zips");
    let raw_dir = std::path::Path::new("data/raw");

    std::fs::create_dir_all(&raw_dir)?;

    for entry in std::fs::read_dir(zips_dir)?
    {
        let entry = entry?;
        let path = entry.path();

        if let Some(ext) = path.extension()
        {
            if ext == "zip"
            {
                let stem = path.file_stem()
                    .and_then(|s| s.to_str())
                    .ok_or("Invalid filename")?;
                
                let file = std::fs::File::open(&path)?;
                let mut archive = zip::ZipArchive::new(file)?;

                if archive.len() == 1   // single file extraction
                {
                    let csv_file = raw_dir.join(format!("{}", stem));
                    if !csv_file.exists()
                    {
                        println!("extracting single file {} to {}", path.display(), csv_file.display());

                        let mut zip_file = archive.by_index(0)?;
                        let mut output_file = std::fs::File::create(&csv_file)?;
                        std::io::copy(&mut zip_file, &mut output_file)?;

                        println!("successfully extracted single file {}", stem);
                    }
                    else 
                    {
                        println!("already extracted single file {}", stem);
                    }
                } 
                else
                {
                    let extract_dir = raw_dir.join(stem);

                    if !extract_dir.exists()
                    {
                        println!("extracting {} to {}", path.display(), extract_dir.display());

                        let file = std::fs::File::open(&path)?;
                        let mut archive = zip::ZipArchive::new(file)?;
                        archive.extract(&extract_dir)?;

                        println!("successfully extracted folder {}", stem);
                    }
                    else
                    {
                        println!("already extracted folder {}", stem);
                    }
                }
            }
        }
    }

    println!("zip extraction complete");
    Ok(())
}