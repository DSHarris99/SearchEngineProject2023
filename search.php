<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Search Results</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            text-align: center;
            padding-top: 50px;
        }
        .container {
            display: inline-block;
            padding: 20px;
            background: #fff;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            margin-bottom: 20px;
        }
        h1 {
            margin-bottom: 30px;
        }
        form {
            margin-bottom: 30px;
        }
        input[type="text"] {
            width: 450px;
            height: 45px;
            padding: 5px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 100%;
        }
        input[type="submit"] {
            height: 50px;
            padding: 5px 25px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 100%;
        }
        input[type="submit"]:hover {
            background-color: #0069d9;
        }
        .results-header {
            font-size: 0.6em; 
            text-align: left; 
            padding-left: 0px; 
        }
        ul {
            list-style-type: none;
            padding: 0;
            text-align: left;
        }
        li {
            background-color: #fff;
            padding: 10px;
            margin-bottom: 10px;
            border-radius: 4px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        a {
            text-decoration: none;
            color: #007bff;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Search Engine</h1>
        <?php
        ini_set('display_errors', 1);
        ini_set('display_startup_errors', 1);
        error_reporting(E_ALL);

        $searchQuery = ""; // Initialize to avoid annoying undefined variable notice

        if ($_SERVER["REQUEST_METHOD"] == "POST") {
            // Collect and clean up input field
            $searchQuery = filter_var($_POST['searchQuery'], FILTER_SANITIZE_STRING);
            $escapedSearchQuery = escapeshellarg($searchQuery);

            // Public path to the query.sh script
            $queryScriptPath = '/home/dsh004/HW5/query.sh';

            // Run query.sh with the search terms and capture any errors
            $command = "$queryScriptPath $escapedSearchQuery 2>&1";
            $output = shell_exec($command);

        }
        ?>

        <form action="search.php" method="post">
            <input type="text" name="searchQuery" placeholder="Enter search terms" value="<?php echo htmlspecialchars($searchQuery); ?>">
            <input type="submit" value="Search">
        </form>
        
        <?php
        // Check if the results file was created and display the results with links
        if ($_SERVER["REQUEST_METHOD"] == "POST" && !empty($searchQuery)) {
            echo "<div class=\"results-header\">";
            echo "<h2>Top results for \"" . htmlspecialchars($searchQuery) . "\":</h2>";
            echo "</div>";
			$resultsFilePath = '/home/dsh004/HW5/queryresults.txt';
			if (file_exists($resultsFilePath)) {
				$lines = file($resultsFilePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
				if (!empty($lines)) {
					echo "<ul>";
					foreach ($lines as $line) {
						list($docName, $weight) = explode(' ', $line, 2);
						$docURL = "http://csce.uark.edu/~dsh004/files/" . $docName;
						$pageTitle = "";

						// attempt to open the document file and retrieve the title
						$docPath = "/home/dsh004/HW5/files/" . $docName;
						if (file_exists($docPath)) {
							$docContent = file_get_contents($docPath);
							if (preg_match('/<title>(.*?)<\/title>/is', $docContent, $matches)) {
								$pageTitle = trim($matches[1]);
								// shorten title if it is longer than 30 characters and add "..."
								if (strlen($pageTitle) > 30) {
									$pageTitle = substr($pageTitle, 0, 30) . "...";
								}
							}
						}

						// in case the document's title is empty or not found
						if (empty($pageTitle)) {
							$pageTitle = "No title available";
						}

						echo "<li><a href=\"" . $docURL . "\">" . htmlspecialchars($docName) . "</a> - " . htmlspecialchars($pageTitle);
						echo "<br><span style=\"font-size:smaller; font-style:italic;\">Score: " . htmlspecialchars($weight) . "</span></li>";
					}
					echo "</ul>";
				} else {
					echo "<p>No results found.</p>";
				}
				unlink($resultsFilePath);
			} else {
				echo "<p>No results found or query.sh did not run correctly.</p>";
			}
		}
		?>
    </div>
</body>
</html>